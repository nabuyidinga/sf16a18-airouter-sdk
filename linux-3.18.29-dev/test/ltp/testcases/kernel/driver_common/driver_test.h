#define MAX_PRINT_MSG_LEN 1024
enum driver_index {
	MTD_INDEX,
	DRIVER_INDEX_END
};
double calc_speed(struct timeval * t1, struct timeval* t2, int data_size);
