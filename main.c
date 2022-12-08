#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <wordexp.h>

#define TEXT_BUF_SIZE 256 * 256
#define EPOCH 1670449116

uint8_t textBuf[TEXT_BUF_SIZE];

static void get_timestamp(uint8_t timestamp[4]) {
	int seconds = (int)time(NULL) - EPOCH;
	int minutes = seconds / 60;
	timestamp[0] = minutes & 0xFF;
	timestamp[1] = (minutes >> 8) & 0xFF;
	timestamp[2] = (minutes >> 16) & 0xFF;
	timestamp[3] = (minutes >> 24) & 0xFF;
}

static void encode_size(size_t size, uint8_t encoded[4]) {
	encoded[0] = size & 0xFF;
	encoded[1] = (size >> 8) & 0xFF;
	encoded[2] = (size >> 16) & 0xFF;
	encoded[3] = (size >> 24) & 0xFF;
}

static int is_valid_utf8(uint8_t buf[TEXT_BUF_SIZE], size_t size) {
	int byte_num = 1;
	for (size_t i = 0; i < size; ++i) {
		if (byte_num == 1 && (buf[i] >> 7) == 0) {
			continue;
		}

		if (byte_num == 1 && (buf[i] >> 5) == 6) {
			byte_num == 2;
			continue;
		}

		if (byte_num == 2 && (buf[i] >> 6) == 2) {
			byte_num = 1;
			continue;
		}

		if (byte_num == 1 && (buf[i] >> 4) == 14) {
			byte_num = 2;
			continue;
		}

		if (byte_num == 2 && (buf[i] >> 6) == 2) {
			byte_num = 3;
			continue;
		}

		if (byte_num == 3 && (buf[i] >> 6) == 2) {
			byte_num = 1;
			continue;
		}

		if (byte_num == 1 && (buf[i] >> 3) == 30) {
			byte_num = 2;
			continue;
		}

		if (byte_num == 2 && (buf[i] >> 6) == 2) {
			byte_num = 3;
			continue;
		}

		if (byte_num == 3 && (buf[i] >> 6) == 2) {
			byte_num = 4;
			continue;
		}

		if (byte_num == 4 && (buf[i] >> 6) == 2) {
			byte_num = 1;
			continue;
		}

		return 0;
	}

	return 1;
}

static char* diary_path() {
	wordexp_t result;
	wordexp("~/.diary", &result, 0);
	return result.we_wordv[0];
}

int main(int argc, char* argv[]) {
	size_t size = fread(textBuf, 1, TEXT_BUF_SIZE, stdin);
	if (!feof(stdin)) {
		fputs("entry was greater than 65kB\n", stderr);
		return -1;
	}
	if (size == 0) {
		fputs("empty entry\n", stderr);
		return -1;
	}

	if (!is_valid_utf8(textBuf, size)) {
		fputs("the diary entry is not valid text\n", stderr);
		return -1;
	}

	uint8_t timestamp[4];
	get_timestamp(timestamp);

	FILE* file = fopen(diary_path(), "ab");
	if (file == NULL) {
		fputs("failed to save: could not open ~/.diary\n", stderr);
		return -1;
	}

	size_t written = fwrite(timestamp, 1, 4, file); 
	if (written != 4) {
		fputs("could not save timestamp\n", stderr);
		return -1;
	}

	uint8_t encoded_size[4];
	encode_size(size, encoded_size);

	written = fwrite(encoded_size, 1, 4, file);
	if (written != 4) {
		fputs("could not save entry size\n", stderr);
		return -1;
	}

	written = fwrite(textBuf, 1, size, file);
	if (written != size) {
		fputs("could not save diary entry\n", stderr);
		return -1;
	}

	return 0;
}
