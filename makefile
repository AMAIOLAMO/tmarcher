BUILD_DIR=build
LIB_DIR=lib
OPTIONS=-Wall -Wextra -O3
INCLUDES=-Ipublic -Ilib/cxvec

.PHONY: clean

$(BUILD_DIR)/tmarcher: tmarcher.cpp
	g++ $(OPTIONS) $(INCLUDES) ./tmarcher.cpp ./private/*.cpp -o $(BUILD_DIR)/tmarcher -lm

clean:
	rm $(BUILD_DIR)/tmarcher
