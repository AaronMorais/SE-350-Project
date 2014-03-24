#include "../syscall.h"

void null_process() {
	while (1) {
		release_processor();
		LOG("Running null process");
	}
}
