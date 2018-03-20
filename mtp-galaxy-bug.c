/*
 * This program is a reproducer for the MTP GetPartialObject offset
 * bug present on Samsung Galaxy devices. This program expects a MTP
 * object ID as argument that corresponds to a file whose size matches
 * the condition (size % 512) == 500. You can invoke the
 * `mtp-galaxy-find-files` script to identify such files on a Samsung
 * Galaxy device.
 *
 * Usage: mtp-galaxy-bug OBJECT_ID
 *
 * SPDX-License-Identifier:	Apache-2.0
 */

#include <libmtp.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static int partial_read(LIBMTP_mtpdevice_t *device,
			uint32_t object_id) {
	LIBMTP_file_t *file;
	uint64_t offset;
	uint32_t count = 512;
	unsigned char data[count];
	unsigned int nr;
	int ret;

	if ((file = LIBMTP_Get_Filemetadata(device, object_id)) == NULL) {
		fprintf(stderr, "Error: cannot get file metadata.\n");
		return -1;
	}

	printf("Object %" PRIu32 ": size = %" PRIu64 " bytes\n", object_id, file->filesize);

	if ((file->filesize % 512) != 500) {
		printf("WARNING: this test expects an object whose size respects 'size %% 512 == 500'\n");
	}
	offset = (file->filesize / 512) * 512;
	printf("GetPartialObject: offset = %" PRIu64 " bytes, count = %" PRIu32 " bytes\n",
	       offset, count);

	ret = LIBMTP_GetPartialObject(device, object_id, offset,
                                      count, (unsigned char **) &data, &nr);
	if (ret == -1) {
		printf("== FAIL ==\n");
		LIBMTP_Dump_Errorstack(device);
		LIBMTP_Clear_Errorstack(device);
	} else {
		printf("== OK (%u/%u) ==\n", nr, count);
	}

	LIBMTP_destroy_file_t(file);

	return ret;
}


int main(int argc, char *argv[])
{
	LIBMTP_mtpdevice_t *device;
	uint32_t object_id;
	LIBMTP_raw_device_t *rawdevices;
	int numrawdevices;
	LIBMTP_error_number_t err;
	char *vendor;
	int ret = -1;

	if (argc != 2 || sscanf(argv[1], "%" PRIu32, &object_id) != 1) {
		fprintf(stderr, "Usage: mtp-galaxy-bug OBJECT_ID\n");
		return EXIT_FAILURE;
	}

	LIBMTP_Init();
	LIBMTP_Set_Debug(2);

	err = LIBMTP_Detect_Raw_Devices(&rawdevices, &numrawdevices);
	if (err) {
		fprintf(stderr, "Error: detect raw devices: %d\n", err);
		goto out;
	}

	if (numrawdevices != 1) {
		fprintf(stderr, "Error: exactly one MTP device is expected.\n");
		goto free_rawdevices;
	}
	device = LIBMTP_Open_Raw_Device_Uncached(&rawdevices[0]);

	vendor = rawdevices[0].device_entry.vendor;
        if (strcmp(vendor, "Samsung") != 0) {
		printf("WARNING: this test expects a Samsung device (vendor = %s)",
		       vendor);
        }
	ret = partial_read(device, object_id);

	LIBMTP_Release_Device(device);
free_rawdevices:
	free(rawdevices);
out:
	return ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
