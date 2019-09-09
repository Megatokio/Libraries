QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        ../Box/BoxP1P2.cpp \
        ../Box/BoxP1SZ.cpp \
        ../Crypt/Crypt.cpp \
        ../LibraryTester/Source/main.cpp \
        ../MemPool/MemPool.cpp \
        ../Names/Names.cpp \
        ../Qt/QEventTypes.cpp \
        ../Templates/Array.test.cpp \
        ../Templates/HashMap.test.cpp \
        ../Templates/RCArray.test.cpp \
        ../Templates/Sort.test.cpp \
        ../Templates/StrArray.test.cpp \
        ../VString/String.cpp \
        ../VString/String_test_suite.cpp \
        ../Var/NameHandles.cpp \
        ../Var/Var.cpp \
        ../Var/test/main.cpp \
        ../Var/var Kopie.cp \
        ../Var/var_test_suite.cp \
        ../Z80/Emu/Z80.cpp \
        ../Z80/goodies/z80_clock_cycles.cpp \
        ../Z80/goodies/z80_major_opcode.cpp \
        ../Z80/goodies/z80_opcode_length.cpp \
        ../audio/AudioDecoder.cpp \
        ../audio/WavFile.cpp \
        ../audio/audio.cpp \
        ../audio/audio_test_helpers.cpp \
        ../cpp/cppthreads.cpp \
        ../cstrings/base85.cpp \
        ../cstrings/cstrings.cpp \
        ../cstrings/cstrings.test.cpp \
        ../cstrings/legacy_charsets.cpp \
        ../cstrings/ucs1.cpp \
        ../cstrings/ucs2.cpp \
        ../cstrings/ucs4.cpp \
        ../cstrings/utf8.cpp \
        ../gif/BoxP1SZ.cpp \
        ../gif/Colormap.cpp \
        ../gif/GifDecoder.cpp \
        ../gif/GifEncoder.cpp \
        ../gif/GifReport.cpp \
        ../gif/Pixelmap.cpp \
        ../hash/crc16.cpp \
        ../hash/md5.cpp \
        ../hash/md5_tools.cpp \
        ../kio/TestTimer.cpp \
        ../kio/exceptions.cpp \
        ../kio/kio.cpp \
        ../rng/MersenneTwister64.cpp \
        ../unix/FD.cpp \
        ../unix/files.cpp \
        ../unix/log.cpp \
        ../unix/n-compress.cpp \
        ../unix/os_utilities.cpp \
        ../unix/pthreads.cpp \
        ../unix/tempmem.cpp \
        ../unix/tempmem.test.cpp \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
	../Box/BoxP1P2.h \
	../Box/BoxP1SZ.h \
	../Crypt/Crypt.h \
	../LibraryTester/Source/custom_errors.h \
	../LibraryTester/Source/settings.h \
	../MemPool/MemPool.h \
	../Names/Names.hpp \
	../Templates/Array.h \
	../Templates/HashMap.h \
	../Templates/NVPtr.h \
	../Templates/RCArray.h \
	../Templates/RCHashMap.h \
	../Templates/RCObject.h \
	../Templates/RCPtr.h \
	../Templates/Stack.h \
	../Templates/StrArray.h \
	../Templates/relational_operators.h \
	../Templates/sort.h \
	../Templates/sorter.h \
	../Templates/template_helpers.h \
	../VString/String.h \
	../VString/String_errors.h \
	../Var/NameHandles.h \
	../Var/Var.h \
	../Var/test/config.h \
	../Var/test/verrors.h \
	../Var/var Kopie.h \
	../Var/var_errors.h \
	../Z80/Emu/Item (template).h \
	../Z80/Emu/Z80.h \
	../Z80/Emu/Z80core.h \
	../Z80/Emu/Z80macros.h \
	../Z80/Emu/Z80opcodes.h \
	../Z80/Emu/Z80options (template).h \
	../Z80/goodies/z80_clock_cycles.h \
	../Z80/goodies/z80_major_opcode.h \
	../Z80/goodies/z80_major_opcode_tables.h \
	../Z80/goodies/z80_opcode_length.h \
	../Z80/goodies/z80_opcodes.h \
	../audio/AudioDecoder.h \
	../audio/CADebugMacros.h \
	../audio/CAStreamBasicDescription.h \
	../audio/WavFile.h \
	../audio/audio.h \
	../audio/audio_test_helpers.h \
	../cpp/cppthreads.h \
	../cstrings/base85.h \
	../cstrings/cstrings.h \
	../cstrings/legacy_charsets.h \
	../cstrings/legacy_charsets/ascii_ger.h \
	../cstrings/legacy_charsets/ascii_us.h \
	../cstrings/legacy_charsets/atari_st.h \
	../cstrings/legacy_charsets/cp_437.h \
	../cstrings/legacy_charsets/html.h \
	../cstrings/legacy_charsets/mac_roman.h \
	../cstrings/legacy_charsets/rtos.h \
	../cstrings/ucs1.h \
	../cstrings/ucs2.h \
	../cstrings/ucs4.h \
	../cstrings/unicode.h \
	../cstrings/unicode/global_constants.h \
	../cstrings/unicode/properties.h \
	../cstrings/unicode/ucs2_blocks.h \
	../cstrings/unicode/ucs2_general_category.h \
	../cstrings/unicode/ucs2_numeric_values.h \
	../cstrings/unicode/ucs2_scripts.h \
	../cstrings/unicode/ucs2_simple_lowercase.h \
	../cstrings/unicode/ucs2_simple_titlecase.h \
	../cstrings/unicode/ucs2_simple_uppercase.h \
	../cstrings/unicode/ucs4_blocks.h \
	../cstrings/unicode/ucs4_general_category.h \
	../cstrings/unicode/ucs4_numeric_values.h \
	../cstrings/unicode/ucs4_scripts.h \
	../cstrings/unicode/ucs4_simple_lowercase.h \
	../cstrings/unicode/ucs4_simple_titlecase.h \
	../cstrings/unicode/ucs4_simple_uppercase.h \
	../cstrings/utf8.h \
	../gif/BoxP1SZ.h \
	../gif/Colormap.h \
	../gif/GifArray.h \
	../gif/GifDecoder.h \
	../gif/GifEncoder.h \
	../gif/GifReport.h \
	../gif/Pixelmap.h \
	../hash/md5.h \
	../hash/md5_tools.h \
	../hash/sdbm_hash.h \
	../kio/TestTimer.h \
	../kio/auto_config.h \
	../kio/detect_configuration.h \
	../kio/errors.h \
	../kio/exceptions.h \
	../kio/kio.h \
	../kio/peekpoke.h \
	../kio/standard_types.h \
	../kio/util/count1bits.h \
	../kio/util/defines.h \
	../kio/util/msbit.h \
	../kio/util/swap.h \
	../rng/MersenneTwister64.h \
	../unix/FD.h \
	../unix/MyFileInfo.h \
	../unix/files.h \
	../unix/log.h \
	../unix/os_utilities.h \
	../unix/pthreads.h \
	../unix/s_type.h \
	../unix/sysctl -A.h \
	../unix/tempmem.h

DISTFILES += \
	../kio/linux_errors.txt
