BUILD_DIR=build
OPTIONS=-Wall -Wextra -O3
INCLUDES=-Ipublic

.PHONY: clean

$(BUILD_DIR)/tmarcher: tmarcher.cpp
	g++ $(OPTIONS) $(INCLUDES) ./tmarcher.cpp ./private/*.cpp -o $(BUILD_DIR)/tmarcher -lm

clean:
	rm $(BUILD_DIR)/tmarcher
