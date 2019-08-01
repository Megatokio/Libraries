TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11
CONFIG += precompiled_header

#LIBS += -pthread

INCLUDEPATH +=  \
	.			\
	Source		\
	Libraries	\


SOURCES += \
	Source/main.cpp \
	Libraries/kio/kio.cpp \
	Libraries/kio/exceptions.cpp \
	Libraries/cstrings/cstrings.cpp \
	Libraries/cstrings/cstrings.test.cpp \
	Libraries/cstrings/utf8.cpp \
	Libraries/cstrings/ucs1.cpp \
	Libraries/cstrings/ucs2.cpp \
	Libraries/cstrings/ucs4.cpp \
	Libraries/cstrings/legacy_charsets.cpp \
	Libraries/unix/FD.cpp \
	Libraries/unix/tempmem.cpp \
	Libraries/unix/tempmem.test.cpp \
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
	config.h \
	Source/settings.h \
	Source/custom_errors.h \
	Libraries/unix/FD.h \
	Libraries/unix/tempmem.h \
	Libraries/kio/errors.h \
	Libraries/kio/exceptions.h \
	Libraries/kio/kio.h \
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



