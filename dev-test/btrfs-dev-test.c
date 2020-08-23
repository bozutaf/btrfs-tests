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

/*
 * Run the following commands before executing the program
 * to setup a test loop device that is gonna be added to
 * the mounted btrfs filesystem /mnt:
 *
 * qemu-img create -f raw test-device.img 1G
 * sudo losetup -f test-disk.img
 *
 * When executing the program, the test devices needs to be
 * specified:
 *
 * ./btrfs-dev-test  /dev/loopX /dev/loopY /dev/loopY1
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
    int dev_control_fd;
    struct btrfs_ioctl_vol_args args = {0};
    struct btrfs_ioctl_vol_args_v2 args_v2 = {0};
    struct btrfs_ioctl_dev_info_args info = {0};
    struct btrfs_ioctl_get_dev_stats stats = {0};

    if (argc < 4) {
        fprintf(stderr, "missing args\n");
        return 1;
    }

    volume_fd = openat(AT_FDCWD, "/mnt", O_RDONLY|O_NONBLOCK
                       |O_CLOEXEC|O_DIRECTORY);

    if (volume_fd < 0) {
        perror("open");
        return 1;
    }

    dev_control_fd = open("/dev/btrfs-control", O_RDWR|O_NONBLOCK);

    if (dev_control_fd < 0) {
        perror("open");
        return 1;
    }

    strncpy(args.name, argv[1], BTRFS_PATH_NAME_MAX);

    if (ioctl(volume_fd, BTRFS_IOC_ADD_DEV, &args) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_ADD_DEV:\n");
    printf("Device %s added to /mnt\n\n", args.name);

    strncpy(args.name, argv[2], BTRFS_PATH_NAME_MAX);

    if (ioctl(volume_fd, BTRFS_IOC_ADD_DEV, &args) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_ADD_DEV:\n");
    printf("Device %s added to /mnt\n\n", args.name);

    strncpy(args.name, argv[3], BTRFS_PATH_NAME_MAX);

    if (ioctl(volume_fd, BTRFS_IOC_ADD_DEV, &args) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_ADD_DEV:\n");
    printf("Device %s added to /mnt\n\n", args.name);

    strncpy(args.name, argv[1], BTRFS_PATH_NAME_MAX);

    if (ioctl(dev_control_fd, BTRFS_IOC_SCAN_DEV, &args) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_SCAN_DEV:\n");
    printf("Device %s contains btrfs filesystem\n\n", args.name);

    info.devid = 2;

    if (ioctl(volume_fd, BTRFS_IOC_DEV_INFO, &info) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_DEV_INFO:\n");
    printf("Bytes used: %llu\n", info.bytes_used);
    printf("Total bytes: %llu\n", info.total_bytes);
    printf("Path: %s\n\n", info.path);

    stats.devid = 2;

    if (ioctl(volume_fd, BTRFS_IOC_GET_DEV_STATS, &stats) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_GET_DEV_STATS:\n");
    printf("nr_items: %llu\n", stats.nr_items);
    printf("flags: %llu\n\n", stats.flags);

    if (ioctl(volume_fd, BTRFS_IOC_RM_DEV, &args) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_RM_DEV:\n");
    if (ioctl(dev_control_fd, BTRFS_IOC_SCAN_DEV, &args) < 0) {
        printf("Device %s removed from /mnt\n\n", args.name);
    }
    else {
        printf("Unable to remove device %s\n\n", args.name);
    }

    args_v2.flags = BTRFS_DEVICE_SPEC_BY_ID;
    args_v2.devid = 3;

    if (ioctl(volume_fd, BTRFS_IOC_RM_DEV_V2, &args_v2) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_RM_DEV_V2:\n");
    if (ioctl(dev_control_fd, BTRFS_IOC_SCAN_DEV, &args) < 0) {
        printf("Device %s removed from /mnt\n\n", argv[2]);
    }
    else {
        printf("Unable to remove device %s\n\n", argv[2]);
    }

    args_v2.flags = 0;
    strncpy(args_v2.name, argv[3], BTRFS_SUBVOL_NAME_MAX);

    if (ioctl(volume_fd, BTRFS_IOC_RM_DEV_V2, &args_v2) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_RM_DEV_V2:\n");
    if (ioctl(dev_control_fd, BTRFS_IOC_SCAN_DEV, &args) < 0) {
        printf("Device %s removed from /mnt\n\n", args.name);
    }
    else {
        printf("Unable to remove device %s\n\n", args.name);
    }

    strncpy(args.name, "", BTRFS_PATH_NAME_MAX);

    if (ioctl(dev_control_fd, BTRFS_IOC_FORGET_DEV, &args) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_FORGET_DEV:\n");
    printf("All unmounted devices removed\n");

    return 0;
}
