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
    Libraries/cstrings/ucs4.h \
    Libraries/kio/peekpoke.h \
    Libraries/Templates/template_helpers.h \
    Libraries/VString/String_errors.h \
    Libraries/VString/String.h \
    Libraries/Unicode/Unicode.h \
    Libraries/Unicode/Includes/Blocks.h \
    Libraries/Unicode/Includes/CanonicalCombiningClass.h \
    Libraries/Unicode/Includes/CanonicalCombiningClassUCS4.h \
    Libraries/Unicode/Includes/CodePointRanges.h \
    Libraries/Unicode/Includes/CodePointRanges_UCS4.h \
    Libraries/Unicode/Includes/EastAsianWidth.h \
    Libraries/Unicode/Includes/EastAsianWidth_UCS4.h \
    Libraries/Unicode/Includes/GeneralCategory.h \
    Libraries/Unicode/Includes/GeneralCategory_UCS4.h \
    Libraries/Unicode/Includes/GlobalConstants.h \
    Libraries/Unicode/Includes/Names.h \
    Libraries/Unicode/Includes/NumericValue.h \
    Libraries/Unicode/Includes/NumericValue_UCS4.h \
    Libraries/Unicode/Includes/Property_Enum.h \
    Libraries/Unicode/Includes/Property_Groups.h \
    Libraries/Unicode/Includes/Property_LongNames.h \
    Libraries/Unicode/Includes/Property_ShortNames.h \
    Libraries/Unicode/Includes/PropertyGroup_Enum.h \
    Libraries/Unicode/Includes/PropertyGroup_Names.h \
    Libraries/Unicode/Includes/PropertyValue_Enum.h \
    Libraries/Unicode/Includes/PropertyValue_LongNames.h \
    Libraries/Unicode/Includes/PropertyValue_ShortNames.h \
    Libraries/Unicode/Includes/Scripts.h \
    Libraries/Unicode/Includes/Scripts_UCS4.h \
    Libraries/Unicode/Includes/SimpleLowercase_Even.h \
    Libraries/Unicode/Includes/SimpleLowercase_Odd.h \
    Libraries/Unicode/Includes/SimpleLowercase_UCS4.h \
    Libraries/Unicode/Includes/SimpleTitlecase.h \
    Libraries/Unicode/Includes/SimpleUppercase_Even.h \
    Libraries/Unicode/Includes/SimpleUppercase_Odd.h \
    Libraries/Unicode/Includes/SimpleUppercase_UCS4.h \
    Libraries/Unicode/Includes/SpecialCasing.h \
    Libraries/Unicode/Includes/UnicodeData_Index.h \
    Libraries/Unicode/Includes/UnicodeData_Names.h \
    Libraries/Unicode/UTF-8.h \



