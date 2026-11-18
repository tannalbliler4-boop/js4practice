#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <new>
#include <string>
#include <sys/resource.h>
#include <unistd.h>

namespace {

// Helper function to safely determine the maximum number of open files.
long get_max_fds() {
    struct rlimit rl {};
    if (getrlimit(RLIMIT_NOFILE, &rl) == 0 && rl.rlim_cur != RLIM_INFINITY) {
        return static_cast<long>(rl.rlim_cur);
    }
    long sc_max = sysconf(_SC_OPEN_MAX);
    return (sc_max > 0) ? sc_max : 1024;
}

} // namespace

// Custom implementation of dup2 (Stevens & Rago, Sec. 3.12) without fcntl().
int my_dup2(int oldfd, int newfd) {
    const long max_fds = get_max_fds();

    // 1. Validate file descriptor bounds
    if (oldfd < 0 || newfd < 0 || oldfd >= max_fds || newfd >= max_fds) {
        errno = EBADF;
        return -1;
    }

    // 2. Verify oldfd is an open, valid file descriptor using dup()
    int test_fd = dup(oldfd);
    if (test_fd < 0) {
        return -1; // errno is already set to EBADF by dup()
    }
    close(test_fd);

    // 3. APUE Sec. 3.12: if oldfd == newfd and oldfd is valid, return newfd
    if (oldfd == newfd) {
        return newfd;
    }

    // 4. Close newfd if it is currently open (preserve errno if unopened)
    int saved_errno = errno;
    close(newfd);
    errno = saved_errno;

    // 5. Dynamically allocate zero-initialized array via smart pointer (RAII)
    const auto array_size = static_cast<std::size_t>(newfd) + 1;
    std::unique_ptr<int[]> temp_fds(new (std::nothrow) int[array_size]());
    if (!temp_fds) {
        errno = ENOMEM;
        return -1;
    }

    int count = 0;
    int current_fd = -1;
    int result = -1;

    // 6. Upper-bounded loop to duplicate oldfd until newfd is reached
    while (count <= newfd) {
        current_fd = dup(oldfd);
        if (current_fd < 0) {
            saved_errno = errno;
            break;
        }

        if (current_fd == newfd) {
            result = current_fd;
            break;
        }

        // Strict bounds check prior to array write
        if (count < newfd) {
            temp_fds[static_cast<std::size_t>(count)] = current_fd;
            ++count;
        } else {
            close(current_fd);
            saved_errno = EBADF;
            break;
        }
    }

    // 7. Close all intermediate file descriptors without altering errno
    const int post_loop_errno = (result < 0) ? saved_errno : errno;
    for (int i = 0; i < count; ++i) {
        close(temp_fds[static_cast<std::size_t>(i)]);
    }

    // Explicitly release dynamic memory early and nullify smart pointer
    temp_fds.reset();

    if (result < 0) {
        errno = post_loop_errno;
    }
    return result;
}

int main() {
    const char *filename = "dup2_test_output.txt";
    const int target_fd = 10;

    // Open test file with restrictive permissions (owner read/write only)
    int orig_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (orig_fd < 0) {
        std::perror("Failed to open test file");
        return 1;
    }

    std::cout << "Opened '" << filename << "' on original fd: " << orig_fd << '\n';

    int dup_fd = my_dup2(orig_fd, target_fd);
    if (dup_fd < 0) {
        std::perror("my_dup2 failed");
        close(orig_fd);
        return 1;
    }

    std::cout << "Successfully duplicated fd " << orig_fd << " to fd " << dup_fd << '\n';

    // --- VALIDITY VERIFICATION SECTION ---
    const std::string msg1 = "Written via original FD.\n";
    const std::string msg2 = "Written via my_dup2 FD.\n";

    if (write(orig_fd, msg1.c_str(), msg1.size()) < 0) {
        std::perror("Write to orig_fd failed");
    }

    // Verify shared file table entry (offset must advance on dup_fd)
    off_t offset_after_first = lseek(dup_fd, 0, SEEK_CUR);
    std::cout << "Offset on dup_fd after writing to orig_fd: "
              << offset_after_first << " (expected " << msg1.size() << ")\n";

    if (write(dup_fd, msg2.c_str(), msg2.size()) < 0) {
        std::perror("Write to dup_fd failed");
    }

    // Close original FD and verify dup_fd remains valid and writable
    close(orig_fd);
    const std::string msg3 = "Written via dup_fd AFTER closing orig_fd.\n";
    if (write(dup_fd, msg3.c_str(), msg3.size()) > 0) {
        std::cout << "Verified: dup_fd (" << dup_fd
                  << ") remains valid after closing orig_fd.\n";
    } else {
        std::perror("dup_fd invalid after closing orig_fd");
    }

    close(dup_fd);

    // --- ERROR HANDLING VERIFICATION ---
    std::cout << "\n--- Testing Error Conditions ---\n";
    if (my_dup2(-1, 5) < 0) {
        std::cout << "Negative oldfd test passed: " << std::strerror(errno) << '\n';
    }
    if (my_dup2(999, 5) < 0) {
        std::cout << "Unopened oldfd test passed: " << std::strerror(errno) << '\n';
    }
    if (my_dup2(STDOUT_FILENO, STDOUT_FILENO) == STDOUT_FILENO) {
        std::cout << "Equal oldfd == newfd test passed: returned "
                  << STDOUT_FILENO << '\n';
    }

    // --- FILE CONTENTS VERIFICATION ---
    std::cout << "\n--- File Contents Verification ---\n";
    std::ifstream infile(filename);
    std::string line;
    while (std::getline(infile, line)) {
        std::cout << "  " << line << '\n';
    }

    return 0;
}
