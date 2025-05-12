# Top level project settings
# These can be overridden in the Gradle File
APP_ABI := armeabi-v7a arm64-v8a x86 x86_64

# Min runtime API level
APP_PLATFORM=android-26

# C++ Settings
APP_STL := c++_static 
APP_CPPFLAGS += -std=c++17
APP_CPPFLAGS += -fexceptions
APP_CPPFLAGS += -frtti
APP_CPPFLAGS += -Wno-conversion-null
APP_CPPFLAGS += -Wno-deprecated 
APP_CPPFLAGS += -Wno-deprecated-declarations

# C Settings
APP_CFLAGS += -Wno-conversion-null
APP_CFLAGS += -Wno-deprecated 
APP_CFLAGS += -Wno-deprecated-declarations
