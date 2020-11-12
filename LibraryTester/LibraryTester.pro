TARGET	= LibraryTester
TEMPLATE = app
QT       -= core
QT       -= gui

CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11
CONFIG += precompiled_header
CONFIG(release,debug|release) { DEFINES += NDEBUG RELEASE } # ATTN: curly brace must start in same line!
CONFIG(debug,debug|release) { DEFINES += DEBUG } # ATTN: curly brace must start in same line!
QMAKE_CXXFLAGS += -Wno-multichar
QMAKE_CXXFLAGS_RELEASE += -O2
QMAKE_CXXFLAGS_DEBUG += -O0

LIBS += -pthread


INCLUDEPATH +=  \
	./			\
	Source		\
	Libraries	\


SOURCES += \
	Libraries/Templates/relational_operators.test.cpp \
	Libraries/Z80/goodies/z80_clock_cycles.cpp \
	Libraries/Z80/goodies/z80_clock_cycles.test.cpp \
	Libraries/Z80/goodies/z80_disass.cpp \
	Libraries/Z80/goodies/z80_disass.test.cpp \
	Libraries/Z80/goodies/z80_major_opcode.test.cpp \
	Libraries/Z80/goodies/z80_opcode_length.cpp \
	Libraries/Z80/goodies/z80_opcode_length.test.cpp \
	Source/main.cpp \
	Libraries/kio/kio.cpp \
	Libraries/kio/kio.test.cpp \
	Libraries/kio/exceptions.cpp \
	Libraries/cstrings/cstrings.cpp \
	Libraries/cstrings/cstrings.test.cpp \
	Libraries/cstrings/utf8.cpp \
	Libraries/cstrings/ucs1.cpp \
	Libraries/cstrings/ucs2.cpp \
	Libraries/cstrings/ucs4.cpp \
	Libraries/cstrings/legacy_charsets.cpp \
	Libraries/unix/FD.cpp \
	Libraries/cstrings/tempmem.cpp \
	Libraries/cstrings/tempmem.test.cpp \
	Libraries/unix/n-compress.cpp \
	\
	Libraries/Templates/Array.test.cpp \
	Libraries/Templates/StrArray.test.cpp \
	Libraries/Templates/HashMap.test.cpp \
	Libraries/Templates/Sort.test.cpp \
	Libraries/Templates/RCArray.test.cpp \
	\
	Libraries/VString/String_test_suite.cpp \
	Libraries/VString/String.cpp

HEADERS += \
	Libraries/Z80/goodies/CpuID.h \
	Libraries/Z80/goodies/z80_goodies.h \
	Libraries/Z80/goodies/z80_opcodes.h \
	Libraries/kio/util/count1bits.h \
	Libraries/kio/util/swap.h \
	Source/main.h \
	Source/settings.h \
	Source/custom_errors.h \
	Libraries/unix/FD.h \
	Libraries/cstrings/tempmem.h \
	Libraries/unix/s_type.h \
	Libraries/kio/errors.h \
	Libraries/kio/exceptions.h \
	Libraries/kio/kio.h \
	Libraries/kio/auto_config.h \
	Libraries/kio/detect_configuration.h \
	Libraries/kio/util/msbit.h \
	Libraries/Templates/Array.h \
	Libraries/Templates/HashMap.h \
	Libraries/Templates/NVPtr.h \
	Libraries/Templates/RCObject.h \
	Libraries/Templates/RCPtr.h \
	Libraries/Templates/sort.h \
	Libraries/Templates/StrArray.h \
	Libraries/Templates/sorter.h \
	Libraries/Templates/relational_operators.h \
	Libraries/kio/standard_types.h \
	Libraries/hash/sdbm_hash.h \
	Libraries/kio/peekpoke.h \
	Libraries/Templates/template_helpers.h \
	Libraries/VString/String_errors.h \
	Libraries/VString/String.h \
	\
	Libraries/cstrings/cstrings.h \
	Libraries/cstrings/utf8.h \
	Libraries/cstrings/ucs1.h \
	Libraries/cstrings/ucs4.h \
	Libraries/cstrings/unicode.h \
	Libraries/cstrings/ucs2.h \
	\
	Libraries/cstrings/legacy_charsets.h \
	Libraries/cstrings/legacy_charsets/ascii_ger.h \
	Libraries/cstrings/legacy_charsets/ascii_us.h \
	Libraries/cstrings/legacy_charsets/atari_st.h \
	Libraries/cstrings/legacy_charsets/cp_437.h \
	Libraries/cstrings/legacy_charsets/html.h \
	Libraries/cstrings/legacy_charsets/mac_roman.h \
	Libraries/cstrings/legacy_charsets/rtos.h \
	\
	Libraries/cstrings/unicode/global_constants.h \
	Libraries/cstrings/unicode/properties.h \
	Libraries/cstrings/unicode/ucs2_blocks.h \
	Libraries/cstrings/unicode/ucs2_general_category.h \
	Libraries/cstrings/unicode/ucs2_numeric_values.h \
	Libraries/cstrings/unicode/ucs2_scripts.h \
	Libraries/cstrings/unicode/ucs2_simple_lowercase.h \
	Libraries/cstrings/unicode/ucs2_simple_titlecase.h \
	Libraries/cstrings/unicode/ucs2_simple_uppercase.h \
	Libraries/cstrings/unicode/ucs4_blocks.h \
	Libraries/cstrings/unicode/ucs4_general_category.h \
	Libraries/cstrings/unicode/ucs4_numeric_values.h \
	Libraries/cstrings/unicode/ucs4_scripts.h \
	Libraries/cstrings/unicode/ucs4_simple_lowercase.h \
	Libraries/cstrings/unicode/ucs4_simple_titlecase.h \
	Libraries/cstrings/unicode/ucs4_simple_uppercase.h



