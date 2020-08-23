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
 * sudo losetup -d /dev/loop0
 */

/*
 * When executing the program the name of the subvolume
 * that is to be created needs to be mentioned.
 *
 * Example execution of the program:
 *
 *  ./btrfs-subvol-test  test_volume  test_volume_v2
 */

int main(int argc, char **argv)
{
    int volume_fd;
    int subvolume_fd;
    struct btrfs_ioctl_vol_args args = {0};
#ifdef BTRFS_IOC_SUBVOL_CREATE_V2
    struct btrfs_ioctl_vol_args_v2 args_v2 = {0};
    struct btrfs_qgroup_inherit inherit = {0};
    struct btrfs_qgroup_limit lim = {0};
    char subvolume_path_v2[BTRFS_PATH_NAME_MAX];
#endif
    char subvolume_path[BTRFS_PATH_NAME_MAX];
    __u64 flags = BTRFS_SUBVOL_RDONLY;
    __u64 default_subvol = 0;
    struct btrfs_ioctl_get_subvol_info_args info = {0};
    struct btrfs_ioctl_get_subvol_rootref_args rootref_args = {0};

    if (argc < 3) {
        fprintf(stderr, "missing args\n");
        return 1;
    }

    volume_fd = openat(AT_FDCWD, "/mnt", O_RDONLY|O_NONBLOCK
                       |O_CLOEXEC|O_DIRECTORY);

    if (volume_fd < 0) {
        perror("open");
        return 1;
    }

    strncpy(args.name, argv[1], BTRFS_PATH_NAME_MAX);

    if (ioctl(volume_fd, BTRFS_IOC_SUBVOL_CREATE, &args) < 0) {
        perror("ioctl");
        return 1;
    }

    strncpy(subvolume_path, "/mnt/", BTRFS_PATH_NAME_MAX);
    strncat(subvolume_path, args.name, BTRFS_PATH_NAME_MAX);

    printf("ioctl BTRFS_SUBVOL_CREATE:\n");
    printf("Successfully created btrfs subvolume: \"%s\"\n\n",subvolume_path);

#ifdef BTRFS_IOC_SUBVOL_CREATE_V2
    strncpy(args.name, argv[2], BTRFS_PATH_NAME_MAX);
    args_v2.flags = BTRFS_SUBVOL_QGROUP_INHERIT;
    args_v2.size = sizeof(inherit) + sizeof(u_int64_t);
    inherit.lim = lim;
    inherit.num_qgroups = 1;
    args_v2.qgroup_inherit = &inherit;

    strncpy(args_v2.name, argv[2], BTRFS_PATH_NAME_MAX);

    if (ioctl(volume_fd, BTRFS_IOC_SUBVOL_CREATE_V2, &args_v2) < 0) {
        perror("ioctl");
        return 1;
    }
#endif

    strncpy(subvolume_path_v2, "/mnt/", BTRFS_PATH_NAME_MAX);
    strncat(subvolume_path_v2, args_v2.name, BTRFS_PATH_NAME_MAX);

    printf("ioctl BTRFS_SUBVOL_CREATE_V2:\n");
    printf("Successfully created btrfs subvolume: \"%s\"\n\n",subvolume_path_v2);

    subvolume_fd = openat(AT_FDCWD, subvolume_path, O_RDONLY|O_NONBLOCK
                          |O_CLOEXEC|O_DIRECTORY);

    if (ioctl(subvolume_fd, BTRFS_IOC_SUBVOL_SETFLAGS, &flags) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_SUBVOL_SETFLAGS:\n");
    printf("Subvolume \"%s\" flags set to BTRFS_SUBVOL_RDONLY.\n\n", subvolume_path);

    if (ioctl(subvolume_fd, BTRFS_IOC_SUBVOL_GETFLAGS, &flags) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_SUBVOL_GETFLAGS:\n");
    if (flags & BTRFS_SUBVOL_RDONLY) {
        printf("Subvolume \"%s\" flags are set to read only.\n\n", subvolume_path);
    }
    else {
        printf("Invalid flags reading!\n\n");
    }

    if (ioctl(subvolume_fd, BTRFS_IOC_GET_SUBVOL_INFO, &info) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_GET_SUBVOL_INFO:\n");
    printf("treeid: %llu\n", info.treeid);
    printf("name: \"%s\"\n", info.name);
    printf("flags: %llu\n", info.flags);
    printf("dirid: %llu\n\n", info.dirid);

    default_subvol = info.treeid;

    if (ioctl(volume_fd, BTRFS_IOC_DEFAULT_SUBVOL, &default_subvol) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_DEFAULT_SUBVOL:\n");
    printf("Subvolume %s set as default\n\n", subvolume_path);

    rootref_args.min_treeid = BTRFS_FIRST_FREE_OBJECTID;

    if (ioctl(volume_fd, BTRFS_IOC_GET_SUBVOL_ROOTREF, &rootref_args) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("ioctl BTRFS_IOC_SUBVOL_ROOTREF:\n");
    printf("Number of found items: %d\n", rootref_args.num_items);

    return 0;
}
