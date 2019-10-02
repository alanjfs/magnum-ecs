#ifndef Corrade_Utility_Debug_h
#define Corrade_Utility_Debug_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

/** @file
 * @brief Class @ref Corrade::Utility::Debug, @ref Corrade::Utility::Warning, @ref Corrade::Utility::Error, @ref Corrade::Utility::Fatal
 */

#include <iosfwd>
#include <string>
#include <tuple>
#include <type_traits>

#include "Corrade/Utility/Macros.h"
#include "Corrade/Containers/EnumSet.h"
#include "Corrade/Utility/TypeTraits.h"
#include "Corrade/Utility/Utility.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

namespace Implementation { struct DebugOstreamFallback; }

/**
@brief Debug output handler

Provides convenient stream interface for passing data to debug output (standard
output). Data are by default separated with spaces and last value is enclosed
with newline character. Example usage:

@snippet Utility.cpp Debug-usage

Support for printing more types can be added by implementing function
@ref operator<<(Debug&, const T&) for given type. If there is no
@cpp operator<<() @ce implemented for printing given type using the @ref Debug
class, suitable @ref std::ostream @cpp operator<<() @ce overload is used as
fallback, if found.

@section Utility-Debug-scoped-output Scoped output redirection

Output specified in class constructor is used for all instances created during
that instance lifetime. @ref Debug, @ref Warning and @ref Error classes outputs
can be controlled separately:

@snippet Utility.cpp Debug-scoped-output

@section Utility-Debug-modifiers Output modifiers

It's possible to modify the debug output by passing a special function to the
output stream.

@subsection Utility-Debug-modifiers-whitespace Explicit whitespace control

Sometimes you might not want to have everything separated by spaces or having
newline at the end:

@snippet Utility.cpp Debug-modifiers-whitespace

@subsection Utility-Debug-modifiers-colors Colored output

It is possible to color the output using @ref color() and @ref boldColor(). The
color is automatically reset to previous value on destruction to avoid messing
up the terminal, you can also use @ref resetColor() to reset it explicitly.

@snippet Utility.cpp Debug-modifiers-colors

@include UtilityDebug-color.ansi

On POSIX the coloring is done using ANSI color escape sequences and works both
when outputting to a terminal or any other stream. On Windows, by default due
to a platform limitation, the colored output works only when outputting
directly to a terminal without any intermediate buffer. See
@ref CORRADE_UTILITY_USE_ANSI_COLORS for possible alternative.

Note that colors make sense only when they finally appear in a terminal and not
when redirecting output to file. You can control this by setting
@ref Flag::DisableColors based on value of @ref isTty(), for example:

@snippet Utility.cpp Debug-modifiers-colors-disable

Similarly as with scoped output redirection, colors can be also scoped:

@snippet Utility.cpp Debug-modifiers-colors-scoped

@see @ref Warning, @ref Error, @ref Fatal, @ref CORRADE_ASSERT(),
    @ref CORRADE_INTERNAL_ASSERT(), @ref CORRADE_INTERNAL_ASSERT_OUTPUT(),
    @ref AndroidLogStreamBuffer, @ref formatString()
@todo Output to more ostreams at once
 */
class CORRADE_UTILITY_EXPORT Debug {
    public:
        /**
         * @brief Debug output flag
         *
         * @see @ref Flags, @ref Debug(Flags)
         */
        enum class Flag: unsigned char {
            /** Don't put newline at the end on destruction */
            NoNewlineAtTheEnd = 1 << 0,

            /**
             * Disable colored output in @ref color(), @ref boldColor() and
             * @ref resetColor().
             * @see @ref isTty()
             * @note Note that on @ref CORRADE_TARGET_WINDOWS "Windows" the
             *      colored output by default works only if outputting directly
             *      to the console. See also @ref CORRADE_UTILITY_USE_ANSI_COLORS.
             */
            DisableColors = 1 << 1
        };

        /**
         * @brief Debug output flags
         *
         * @see @ref Debug(Flags)
         */
        typedef Containers::EnumSet<Flag> Flags;

        /** @{ @name Output modifiers
         * See @ref Utility-Debug-modifiers for more information.
         */

        /**
         * @brief Debug output modifier
         *
         * @see @ref nospace(), @ref newline(), @ref operator<<(Modifier)
         */
        typedef void(*Modifier)(Debug&);

        /**
         * @brief Output color
         *
         * @see @ref color(), @ref boldColor()
         */
        enum class Color: char {
            /** Black */
            Black = 0,

            /** Red */
            #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
            Red = 1,
            #else
            Red = 4,
            #endif

            /** Green */
            Green = 2,

            /** Yellow */
            #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
            Yellow = 3,
            #else
            Yellow = 6,
            #endif

            /** Blue */
            #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
            Blue = 4,
            #else
            Blue = 1,
            #endif

            /** Magenta */
            Magenta = 5,

            /** Cyan */
            #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
            Cyan = 6,
            #else
            Cyan = 3,
            #endif

            /** White */
            White = 7,

            /** Default (implementation/style-defined) */
            #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
            Default = 9
            #else
            Default = 7
            #endif
        };

        /**
         * @brief Don't put space before next value
         *
         * Debug output by default separates values with space, this disables
         * it for the immediately following value. The default behavior is
         * then restored. The following line outputs
         * @cb{.shell-session} Value: 16, 24 @ce:
         *
         * @snippet Utility.cpp Debug-nospace
         *
         * @see @ref newline()
         */
        /* MinGW complains loudly if the declaration doesn't also have inline */
        inline static void nospace(Debug& debug);

        /**
         * @brief Output a newline
         *
         * Puts a newline (not surrounded by spaces) to the output. The
         * following two lines are equivalent:
         *
         * @snippet Utility.cpp Debug-newline
         *
         * and their output is
         *
         * @code{.shell-session}
         * Value:
         * 16
         * @endcode
         *
         * @see @ref nospace()
         */
        static void newline(Debug& debug) {
            debug << nospace << "\n" << nospace;
        }

        /**
         * @brief Set output color
         *
         * Resets previous @ref color() or @ref boldColor() setting. The color
         * is also automatically reset on object destruction to a value that
         * was active in outer scope. If @ref Flag::DisableColors was set, this
         * function does nothing.
         */
        static Modifier color(Color color);

        /**
         * @brief Set bold output color
         *
         * Resets previous @ref color() or @ref boldColor() setting. The color
         * is also automatically reset on object destruction to a value that
         * was active in outer scope. If @ref Flag::DisableColors was set, this
         * function does nothing.
         */
        static Modifier boldColor(Color color);

        /**
         * @brief Reset output color
         *
         * Resets any previous @ref color() or @ref boldColor() setting to a
         * value that was active in outer scope. The same is also automatically
         * done on object destruction. If the color was not changed by this
         * instance or @ref Flag::DisableColors was set, this function does
         * nothing.
         */
        static void resetColor(Debug& debug);

        /**
         * @brief Debug output modification
         *
         * See @ref Utility-Debug-modifiers for more information.
         */
        Debug& operator<<(Modifier f) {
            f(*this);
            return *this;
        }

        /*@}*/

        /**
         * @brief Current debug output stream
         *
         * Debug output constructed with the @ref Debug(Flags) constructor will
         * be using this output stream.
         */
        static std::ostream* output();

        /**
         * @brief Whether given output stream is a TTY
         *
         * Useful for deciding whether to use ANSI colored output using
         * @ref Flag::DisableColors. Returns @cpp true @ce if @p output is a
         * pointer to @ref std::cout / @ref std::cerr and the stream is not
         * redirected to a file, @cpp false @ce otherwise. Calls @cpp isatty() @ce
         * on Unix-like systems and Windows with @ref CORRADE_UTILITY_USE_ANSI_COLORS
         * enabled, calls Windows APIs if @ref CORRADE_UTILITY_USE_ANSI_COLORS
         * is disabled. On platforms without @cpp isatty() @ce equivalent
         * returns always @cpp false @ce.
         *
         * @note Returns @cpp false @ce when running inside Xcode even though
         *      @cpp isatty() @ce reports a positive value, because Xcode is
         *      not able to handle ANSI colors inside the output view.
         * @note Uses Node.js @cb{.js} process.stdout.isTTY @ce /
         *      @cb{.js} process.stderr.isTTY @ce instead of @cpp isatty() @ce
         *      on @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten" because
         *      @cpp isatty() @ce is not able to detect file redirection.
         */
        static bool isTty(std::ostream* output);

        /**
         * @brief Whether current debug output is a TTY
         *
         * Calls @ref isTty(std::ostream*) with output of enclosing @ref Debug
         * instance or with @ref std::cerr if there isn't any.
         * @see @ref Warning::isTty(), @ref Error::isTty()
         */
        static bool isTty();

        /**
         * @brief Default constructor
         * @param flags         Output flags
         *
         * Uses output of enclosing @ref Debug instance or uses @ref std::cout
         * if there isn't any.
         */
        explicit Debug(Flags flags = {});

        /**
         * @brief Constructor
         * @param output        Stream where to put debug output. If set to
         *      @cpp nullptr @ce, no debug output will be written anywhere.
         * @param flags         Output flags
         *
         * All new instances created using the default @ref Debug() constructor
         * during lifetime of this instance will inherit the output set in
         * @p output.
         */
        explicit Debug(std::ostream* output, Flags flags = {});

        /** @brief Copying is not allowed */
        Debug(const Debug&) = delete;

        /** @brief Move constructor */
        Debug(Debug&&) = default;

        /**
         * @brief Destructor
         *
         * Resets the output redirection back to the output of enclosing scope.
         * If there was any output, adds newline at the end. Also resets output
         * color modifier, if there was any.
         * @see @ref resetColor()
         */
        ~Debug();

        /** @brief Copying is not allowed */
        Debug& operator=(const Debug&) = delete;

        /** @brief Move assignment is not allowed */
        Debug& operator=(Debug&&) = delete;

        /**
         * @brief Print string to debug output
         *
         * If there is already something in the output, puts a space before
         * the value, unless @ref nospace() was set immediately before.
         * @see @ref operator<<(Debug&, const T&)
         */
        Debug& operator<<(const std::string& value);
        Debug& operator<<(const char* value);            /**< @overload */
        Debug& operator<<(const void* value);            /**< @overload */
        Debug& operator<<(bool value);                   /**< @overload */
        Debug& operator<<(int value);                    /**< @overload */
        Debug& operator<<(long value);                   /**< @overload */
        Debug& operator<<(long long value);              /**< @overload */
        Debug& operator<<(unsigned value);               /**< @overload */
        Debug& operator<<(unsigned long value);          /**< @overload */
        Debug& operator<<(unsigned long long value);     /**< @overload */

        /**
         * @brief Print `float` value to debug output
         *
         * Prints the value with 6 significant digits.
         */
        Debug& operator<<(float value);

        /**
         * @brief Print `double` value to debug output
         *
         * Prints the value with 15 significant digits.
         */
        Debug& operator<<(double value);

        #ifndef CORRADE_TARGET_EMSCRIPTEN
        /**
         * @brief Print `long double` value to debug output
         *
         * Prints the value with 18 significant digits.
         * @partialsupport Not available in @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten"
         *      as JavaScript doesn't support doubles larger than 64 bits.
         */
        Debug& operator<<(long double value);
        #endif

        /**
         * @brief Print UTF-32 character to debug output
         *
         * Prints the value as Unicode codepoint, i.e. @cb{.shell-session} U+0061 @ce.
         */
        Debug& operator<<(char32_t value);

        /**
         * @brief Print UTF-32 character literal to debug output
         *
         * Prints the value as list of Unicode codepoints, i.e.
         * @cb{.shell-session} {U+0061, U+0062, U+0063} @ce.
         */
        Debug& operator<<(const char32_t* value);

        /**
         * @brief Print a nullptr to debug output
         *
         * Prints the value as @cb{.shell-session} nullptr @ce.
         */
        Debug& operator<<(std::nullptr_t);

        #ifndef DOXYGEN_GENERATING_OUTPUT
        Debug& operator<<(Implementation::DebugOstreamFallback&& value);
        #endif

    #ifndef DOXYGEN_GENERATING_OUTPUT
    protected:
    #else
    private:
    #endif
        std::ostream* _output;

        enum class InternalFlag: unsigned char {
            /* Values compatible with Flag enum */
            NoNewlineAtTheEnd = 1 << 0,
            DisableColors = 1 << 1,
            NoSpaceBeforeNextValue = 1 << 2,
            ValueWritten = 1 << 3,
            ColorWritten = 1 << 4
        };
        typedef Containers::EnumSet<InternalFlag> InternalFlags;

        CORRADE_ENUMSET_FRIEND_OPERATORS(InternalFlags)

        CORRADE_UTILITY_LOCAL void cleanupOnDestruction(); /* Needed for Fatal */

        InternalFlags _flags;

    private:
        template<Color c, bool bold> CORRADE_UTILITY_LOCAL static Modifier colorInternal();

        static CORRADE_UTILITY_LOCAL std::ostream* _globalOutput;
        #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
        static CORRADE_UTILITY_LOCAL Color _globalColor;
        static CORRADE_UTILITY_LOCAL bool _globalColorBold;
        #endif

        template<class T> CORRADE_UTILITY_LOCAL Debug& print(const T& value);
        CORRADE_UTILITY_LOCAL void resetColorInternal();

        std::ostream* _previousGlobalOutput;
        #if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_UTILITY_USE_ANSI_COLORS)
        unsigned short _previousColorAttributes = 0xffff;
        #else
        Color _previousColor;
        bool _previousColorBold;
        #endif
};

CORRADE_ENUMSET_OPERATORS(Debug::Flags)
CORRADE_ENUMSET_OPERATORS(Debug::InternalFlags)

inline void Debug::nospace(Debug& debug) {
    debug._flags |= InternalFlag::NoSpaceBeforeNextValue;
}

#ifndef DOXYGEN_GENERATING_OUTPUT
/* so Debug() << value works */
template<class T> inline Debug& operator<<(Debug&& debug, const T& value) {
    return debug << value;
}
#endif

#ifdef DOXYGEN_GENERATING_OUTPUT
/** @relates Debug
@brief Operator for printing custom types to debug
@param debug     Debug class
@param value     Value to be printed

Support for printing custom types (i.e. those not handled by @ref std::iostream)
can be added by implementing this function for given type.

The function should convert the type to one of supported types (such as
@ref std::string) and then call @ref Debug::operator<<(const std::string&) with
it. You can also use @ref Debug::nospace() and @ref Debug::newline().
 */
template<class T> Debug& operator<<(Debug& debug, const T& value);
#endif

/** @relates Debug
@brief Operator for printing iterable types to debug

Prints the value as @cb{.shell-session} {a, b, c} @ce.
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
template<class Iterable> Debug& operator<<(Debug& debug, const Iterable& value)
#else
/* libc++ from Apple's Clang "4.2" (3.2-svn) doesn't have constexpr operator
   bool for std::integral_constant, thus we need to use ::value instead */
template<class Iterable> Debug& operator<<(typename std::enable_if<IsIterable<Iterable>::value && !std::is_same<Iterable, std::string>::value, Debug&>::type debug, const Iterable& value)
#endif
{
    debug << "{" << Debug::nospace;
    for(auto it = value.begin(); it != value.end(); ++it) {
        if(it != value.begin())
            debug << Debug::nospace << ",";
        debug << *it;
    }
    debug << Debug::nospace << "}";
    return debug;
}

namespace Implementation {
    /** @todo C++14: use std::make_index_sequence and std::integer_sequence */
    template<std::size_t ...> struct Sequence {};

    #ifndef DOXYGEN_GENERATING_OUTPUT
    /* E.g. GenerateSequence<3>::Type is Sequence<0, 1, 2> */
    template<std::size_t N, std::size_t ...sequence> struct GenerateSequence:
        GenerateSequence<N-1, N-1, sequence...> {};

    template<std::size_t ...sequence> struct GenerateSequence<0, sequence...> {
        typedef Sequence<sequence...> Type;
    };
    #endif

    /* Used by operator<<(Debug&, std::tuple<>...) */
    template<class T> inline void tupleDebugOutput(Debug&, const T&, Sequence<>) {}
    template<class T, std::size_t i, std::size_t ...sequence> void tupleDebugOutput(Debug& debug, const T& tuple, Sequence<i, sequence...>) {
        debug << std::get<i>(tuple);
        #ifdef _MSC_VER
        #pragma warning(push)
        #pragma warning(disable:4127) /* conditional expression is constant (of course) */
        #endif
        if(i + 1 != std::tuple_size<T>::value)
            debug << Debug::nospace << ",";
        #ifdef _MSC_VER
        #pragma warning(pop)
        #endif
        tupleDebugOutput(debug, tuple, Sequence<sequence...>{});
    }
}

/** @relates Debug
@brief Operator for printing tuple types to debug

Prints the value as @cb{.shell-session} (first, second, third...) @ce.
*/
template<class ...Args> Debug& operator<<(Debug& debug, const std::tuple<Args...>& value) {
    debug << "(" << Debug::nospace;
    Implementation::tupleDebugOutput(debug, value, typename Implementation::GenerateSequence<sizeof...(Args)>::Type{});
    debug << Debug::nospace << ")";
    return debug;
}

/** @relates Debug
 * @overload
 */
template<class T, class U> Debug& operator<<(Debug& debug, const std::pair<T, U>& value) {
    return debug << "(" << Debug::nospace << value.first << Debug::nospace << "," << value.second << Debug::nospace << ")";
}

/**
@brief Warning output handler

Same as @ref Debug, but by default writes output to standard error output.
Thus it is possible to separate / mute @ref Debug, @ref Warning and @ref Error
outputs.
*/
class CORRADE_UTILITY_EXPORT Warning: public Debug {
    public:
        /**
         * @brief Current warning output stream
         *
         * Warning output constructed with the @ref Warning(Flags) constructor
         * will be using this output stream.
         */
        static std::ostream* output();

        /**
         * @brief Whether current warning output is a TTY
         *
         * Calls @ref isTty(std::ostream*) with output of enclosing
         * @ref Warning instance or with @ref std::cerr if there isn't any.
         * @see @ref Debug::isTty(), @ref Error::isTty()
         */
        static bool isTty();

        /**
         * @brief Default constructor
         * @param flags         Output flags
         *
         * Inherits output of enclosing @ref Warning instance or uses
         * @ref std::cerr if there isn't any.
         */
        explicit Warning(Flags flags = {});

        /**
         * @brief Constructor
         * @param output        Stream where to put warning output. If set to
         *      @cpp nullptr @ce, no warning output will be written anywhere.
         * @param flags         Output flags
         *
         * All new instances created using the default @ref Warning()
         * constructor during lifetime of this instance will inherit the output
         * set in @p output.
         */
        explicit Warning(std::ostream* output, Flags flags = {});

        /** @brief Copying is not allowed */
        Warning(const Warning&) = delete;

        /** @brief Move constructor */
        Warning(Warning&&) = default;

        /**
         * @brief Destructor
         *
         * Resets the output redirection back to the output of enclosing scope.
         * If there was any output, adds newline at the end. Also resets output
         * color modifier, if there was any.
         * @see @ref resetColor()
         */
        ~Warning();

        /** @brief Copying is not allowed */
        Warning& operator=(const Warning&) = delete;

        /** @brief Move assignment is not allowed */
        Warning& operator=(Warning&&) = delete;

    private:
        static CORRADE_UTILITY_LOCAL std::ostream* _globalWarningOutput;
        std::ostream* _previousGlobalWarningOutput;
};

/**
@brief Error output handler

@copydetails Warning
@see @ref Fatal
*/
class CORRADE_UTILITY_EXPORT Error: public Debug {
    friend Fatal;

    public:
        /**
         * @brief Current error output stream
         *
         * Error output constructed with the @ref Error(Flags) constructor
         * will be using this output stream.
         */
        static std::ostream* output();

        /**
         * @brief Whether current error output stream is a TTY
         *
         * Calls @ref isTty(std::ostream*) with output of enclosing @ref Error
         * instance or with @ref std::cerr if there isn't any.
         * @see @ref Debug::isTty(), @ref Warning::isTty()
         */
        static bool isTty();

        /**
         * @brief Default constructor
         * @param flags         Output flags
         *
         * Inherits output of enclosing @ref Error instance or uses
         * @ref std::cerr if there isn't any.
         */
        explicit Error(Flags flags = {});

        /**
         * @brief Constructor
         * @param output        Stream where to put error output. If set to
         *      @cpp nullptr @ce, no error output will be written anywhere.
         * @param flags         Output flags
         *
         * All new instances created using the default @ref Error()
         * constructor during lifetime of this instance will inherit the output
         * set in @p output.
         */
        explicit Error(std::ostream* output, Flags flags = {});

        /** @brief Copying is not allowed */
        Error(const Error&) = delete;

        /** @brief Move constructor */
        Error(Error&&) = default;

        /**
         * @brief Destructor
         *
         * Resets the output redirection back to the output of enclosing scope.
         * If there was any output, adds newline at the end. Also resets output
         * color modifier, if there was any.
         * @see @ref resetColor()
         */
        ~Error();

        /** @brief Copying is not allowed */
        Error& operator=(const Error&) = delete;

        /** @brief Move assignment is not allowed */
        Error& operator=(Error&&) = delete;

    #ifndef DOXYGEN_GENERATING_OUTPUT
    protected:
    #else
    private:
    #endif
        CORRADE_UTILITY_LOCAL void cleanupOnDestruction(); /* Needed for Fatal */

    private:
        static CORRADE_UTILITY_LOCAL std::ostream* _globalErrorOutput;
        std::ostream* _previousGlobalErrorOutput;
};

/**
@brief Warning output handler

Equivalent to @ref Error, but exits with defined exit code on destruction. So
instead of this:

@snippet Utility.cpp Fatal-Error

You can write just this:

@snippet Utility.cpp Fatal-Fatal
*/
class CORRADE_UTILITY_EXPORT Fatal: public Error {
    public:
        /**
         * @brief Constructor
         * @param exitCode      Application exit code to be used on destruction
         * @param flags         Output flags
         *
         * Sets output to @ref std::cerr. The @p exitCode is passed to
         * @ref std::exit() on destruction.
         */
        Fatal(int exitCode = 1, Flags flags = {}): Error{flags}, _exitCode{exitCode} {}

        /** @overload */
        Fatal(Flags flags): Fatal{1, flags} {}

        /**
         * @brief Constructor
         * @param output        Stream where to put debug output. If set to
         *      @cpp nullptr @ce, no debug output will be written anywhere.
         * @param exitCode      Application exit code to be used on destruction
         * @param flags         Output flags
         */
        Fatal(std::ostream* output, int exitCode = 1, Flags flags = {}): Error{output, flags}, _exitCode{exitCode} {}

        /** @overload */
        Fatal(std::ostream* output, Flags flags = {}): Fatal{output, 1, flags} {}

        /**
         * @brief Destructor
         *
         * Exits the application with exit code specified in constructor.
         */
        #ifndef CORRADE_MSVC2015_COMPATIBILITY
        /* http://stackoverflow.com/questions/38378693/did-visual-studio-2015-update-3-break-constructor-attributes */
        CORRADE_NORETURN
        #endif
        ~Fatal();

    private:
        int _exitCode;
};

/** @debugoperatorclassenum{Debug,Debug::Color} */
CORRADE_UTILITY_EXPORT Debug& operator<<(Debug& debug, Debug::Color value);

namespace Implementation {

/* Used by Debug::operator<<(Implementation::DebugOstreamFallback&&) */
struct DebugOstreamFallback {
    template<class T> /*implicit*/ DebugOstreamFallback(const T& t): applier(&DebugOstreamFallback::applyImpl<T>), value(&t) {}

    void apply(std::ostream& s) const {
        (this->*applier)(s);
    }

    template<class T> void applyImpl(std::ostream& s) const {
        s << *static_cast<const T*>(value);
    }

    using ApplierFunc = void(DebugOstreamFallback::*)(std::ostream&) const;
    const ApplierFunc applier;
    const void* value;
};

}

}}

#endif
