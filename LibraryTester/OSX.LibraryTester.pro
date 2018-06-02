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
    Libraries/unix/FD.cpp \
    Libraries/unix/tempmem.cpp \
    Libraries/unix/tempmem.test.cpp \
    \
    Libraries/Templates/Array.test.cpp \
    Libraries/Templates/StrArray.test.cpp \
    Libraries/Templates/HashMap.test.cpp \
    Libraries/Templates/Sort.test.cpp \
    Libraries/Templates/RCArray.test.cpp \
    \
    Libraries/VString/String_test_suite.cpp \
    Libraries/VString/String.cpp \
    Libraries/Unicode/Unicode.cpp \
    Libraries/Unicode/UnicodeLegacyCharsets.cpp \
    Libraries/Unicode/UTF-8.cpp

HEADERS += \
    config.h \
    Source/settings.h \
    Source/custom_errors.h \
    Libraries/unix/FD.h \
    Libraries/unix/tempmem.h \
    Libraries/cstrings/cstrings.h \
    Libraries/kio/error_emacs.h \
    Libraries/kio/errors.h \
    Libraries/kio/exceptions.h \
    Libraries/kio/kio.h \
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
    Libraries/cstrings/utf8.h \
    Libraries/cstrings/ucs1.h \
    Libraries/kio/peekpoke.h \
    Libraries/Templates/template_helpers.h \
    Libraries/VString/String_errors.h \
    Libraries/VString/String.h \



