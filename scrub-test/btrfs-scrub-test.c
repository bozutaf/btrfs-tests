#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/btrfs.h>
#include <string.h>

/*
 * Run the following commands before executing the program
 * to setup a test loop device with btrfs filesystem:
 *
 * qemu-img create -f raw test-disk.img 1G
 * sudo losetup -f test-disk.img
 * sudo mkfs -t btrfs /dev/loopX
 * sudo mount /dev/loop0 /mnt
 *
 * After finishing with the loop device, run following commands
 * for cleanup:
 *
 * sudo umount /mnt
 * sudo losetup -d /dev/loopX
 */

int main(int argc, char **argv)
{
    int volume_fd;
    struct btrfs_ioctl_scrub_args scrub_args = {0};

    volume_fd = openat(AT_FDCWD, "/mnt", O_RDONLY|O_NONBLOCK
                       |O_CLOEXEC|O_DIRECTORY);

    if (volume_fd < 0) {
        perror("open");
        return 1;
    }

    scrub_args.devid = 1;
    scrub_args.flags = BTRFS_SCRUB_READONLY;

    if (ioctl(volume_fd, BTRFS_IOC_SCRUB, &scrub_args) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_SCRUB:\n");
    printf("Scrubbing of filesystem /mnt started.\n\n");

    if (ioctl(volume_fd, BTRFS_IOC_SCRUB_PROGRESS, &scrub_args) < 0) {
        perror("ioctl BTRFS_IOC_SCRUB_PROGRESS");
        printf("\n");
    }

    if (ioctl(volume_fd, BTRFS_IOC_SCRUB_CANCEL, &scrub_args) < 0) {
        perror("ioctl BTRFS_IOC_SCRUB_CANCEL");
        printf("\n");
    }

    return 0;
}
