#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/btrfs.h>
#include <linux/btrfs_tree.h>
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

/*
 * Create a test subvolume with the following command:
 *
 * sudo btrfs subvol create /mnt/test_subvol
 *
 * Whene executing the program the test subvolume needs to be
 * specified:
 *
 * ./ino-test  test_subvol
 *
 */

int main(int argc, char **argv)
{
    int volume_fd, subvolume_fd;
    char subvolume_path[BTRFS_PATH_NAME_MAX];
    struct btrfs_ioctl_ino_lookup_args lookup_args = {0};
    struct btrfs_ioctl_ino_lookup_user_args lookup_user_args = {0};
    struct btrfs_ioctl_get_subvol_info_args info = {0};
    struct btrfs_ioctl_ino_path_args path_args = {0};
    struct btrfs_ioctl_logical_ino_args logical_args = {0};

    if (argc < 2) {
        fprintf(stderr, "missing args\n");
        return 1;
    }

    volume_fd = openat(AT_FDCWD, "/mnt", O_RDONLY|O_NONBLOCK
                       |O_CLOEXEC|O_DIRECTORY);

    if (volume_fd < 0) {
        perror("open");
        return 1;
    }

    strncpy(subvolume_path, "/mnt/", BTRFS_PATH_NAME_MAX);
    strncat(subvolume_path, argv[1], BTRFS_PATH_NAME_MAX);
    subvolume_fd = openat(AT_FDCWD, subvolume_path, O_RDONLY|O_NONBLOCK
                          |O_CLOEXEC|O_DIRECTORY);

    lookup_args.treeid = 0;
    lookup_args.objectid = BTRFS_FIRST_FREE_OBJECTID;

    if (ioctl(volume_fd, BTRFS_IOC_INO_LOOKUP, &lookup_args) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_INO_LOOKUP:\n");
    printf("treeid: %llu\n", lookup_args.treeid);
    printf("name: %s\n\n", lookup_args.name);

    if (ioctl(subvolume_fd, BTRFS_IOC_GET_SUBVOL_INFO, &info) < 0) {
        perror("ioctl");
        return 1;
    }

    lookup_user_args.dirid = info.dirid;
    lookup_user_args.treeid = info.treeid;

    if (ioctl(volume_fd, BTRFS_IOC_INO_LOOKUP_USER, &lookup_user_args) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_INO_LOOKUP_USER:\n");
    printf("name: %s\n", lookup_user_args.name);
    printf("path: %s\n\n", lookup_user_args.path);

    path_args.inum = info.dirid;
    path_args.size = 0;

    if (ioctl(subvolume_fd, BTRFS_IOC_INO_PATHS, &path_args) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_INO_PATHS:\n");
    printf("fspath: %llu\n\n", path_args.fspath);

    logical_args.logical = -1;
    logical_args.size = 0;

    if (ioctl(volume_fd, BTRFS_IOC_LOGICAL_INO, &logical_args) < 0) {
        perror("ioctl BTRFS_IOC_LOGICAL_INO");
    }

    logical_args.flags = BTRFS_LOGICAL_INO_ARGS_IGNORE_OFFSET;

    if (ioctl(volume_fd, BTRFS_IOC_LOGICAL_INO_V2, &logical_args) < 0) {
        perror("ioctl BTRFS_IOC_LOGICAL_INO_V2");
    }

    return 0;
}