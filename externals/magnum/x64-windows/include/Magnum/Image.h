#ifndef Magnum_Image_h
#define Magnum_Image_h
/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019
              Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Class @ref Magnum::Image, @ref Magnum::CompressedImage, typedef @ref Magnum::Image1D, @ref Magnum::Image2D, @ref Magnum::Image3D, @ref Magnum::CompressedImage1D, @ref Magnum::CompressedImage2D, @ref Magnum::CompressedImage3D
 */

#include <Corrade/Containers/Array.h>

#include "Magnum/ImageView.h"

namespace Magnum {

/**
@brief Image

Stores multi-dimensional image data together with layout and pixel format
description. See @ref ImageView for a non-owning alternative.

This class can act as a drop-in replacement for @ref ImageView and
@ref Trade::ImageData APIs and is implicitly convertible to @ref ImageView.
Particular graphics API wrappers provide additional image classes, for example
@ref GL::BufferImage. See also @ref CompressedImage for equivalent
functionality targeted on compressed image formats.

@section Image-usage Basic usage

The image takes ownership of a passed @ref Corrade::Containers::Array, together
with storing image size and one of the generic @ref PixelFormat values:

@snippet Magnum.cpp Image-usage

On construction, the image internally calculates pixel size corresponding to
given pixel format using @ref pixelSize(). This value is needed to check that
the passed data array is large enough and is also required by most image
manipulation operations.

It's also possible to create just an image placeholder, storing only the image
properties without data or size. That is useful for example to specify desired
format of image queries in graphics APIs:

@snippet Magnum.cpp Image-usage-query

As with @ref ImageView, this class supports extra storage parameters and
implementation-specific pixel format specification. See the @ref ImageView
documentation for more information.

@see @ref Image1D, @ref Image2D, @ref Image3D, @ref CompressedImage
*/
template<UnsignedInt dimensions> class Image {
    public:
        enum: UnsignedInt {
            Dimensions = dimensions /**< Image dimension count */
        };

        /**
         * @brief Constructor
         * @param storage           Storage of pixel data
         * @param format            Format of pixel data
         * @param size              Image size
         * @param data              Image data
         *
         * The @p data array is expected to be of proper size for given
         * parameters.
         */
        explicit Image(PixelStorage storage, PixelFormat format, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data) noexcept;

        /**
         * @brief Constructor
         * @param format            Format of pixel data
         * @param size              Image size
         * @param data              Image data
         *
         * Equivalent to calling @ref Image(PixelStorage, PixelFormat, const VectorTypeFor<dimensions, Int>&, Containers::Array<char>&&)
         * with default-constructed @ref PixelStorage.
         */
        explicit Image(PixelFormat format, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data) noexcept: Image{{}, format, size, std::move(data)} {}

        /**
         * @brief Construct an image placeholder
         * @param storage           Storage of pixel data
         * @param format            Format of pixel data
         *
         * Size is set to zero and data pointer to @cpp nullptr @ce. Move over
         * a non-empty instance to make it useful.
         */
        /*implicit*/ Image(PixelStorage storage, PixelFormat format) noexcept;

        /**
         * @brief Construct an image placeholder
         * @param format            Format of pixel data
         *
         * Equivalent to calling @ref Image(PixelStorage, PixelFormat)
         * with default-constructed @ref PixelStorage.
         */
        /*implicit*/ Image(PixelFormat format) noexcept: Image{{}, format} {}

        /**
         * @brief Construct an image with implementation-specific pixel format
         * @param storage           Storage of pixel data
         * @param format            Format of pixel data
         * @param formatExtra       Additional pixel format specifier
         * @param pixelSize         Size of a pixel in given format
         * @param size              Image size
         * @param data              Image data
         *
         * Unlike with @ref Image(PixelStorage, PixelFormat, const VectorTypeFor<dimensions, Int>&, Containers::Array<char>&&),
         * where pixel size is calculated automatically using
         * @ref pixelSize(PixelFormat), this allows you to specify an
         * implementation-specific pixel format and pixel size directly. Uses
         * @ref pixelFormatWrap() internally to wrap @p format in
         * @ref PixelFormat.
         *
         * The @p data array is expected to be of proper size for given
         * parameters.
         */
        explicit Image(PixelStorage storage, UnsignedInt format, UnsignedInt formatExtra, UnsignedInt pixelSize, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data) noexcept;

        /** @overload
         *
         * Equivalent to the above for @p format already wrapped with
         * @ref pixelFormatWrap().
         */
        explicit Image(PixelStorage storage, PixelFormat format, UnsignedInt formatExtra, UnsignedInt pixelSize, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data) noexcept;

        /**
         * @brief Construct an image placeholder with implementation-specific pixel format
         * @param storage           Storage of pixel data
         * @param format            Format of pixel data
         * @param formatExtra       Additional pixel format specifier
         * @param pixelSize         Size of a pixel in given format
         *
         * Unlike with @ref Image(PixelStorage, PixelFormat), where pixel size
         * is calculated automatically using @ref pixelSize(PixelFormat), this
         * allows you to specify an implementation-specific pixel format and
         * pixel size directly. Uses @ref pixelFormatWrap() internally to
         * wrap @p format in @ref PixelFormat.
         */
        explicit Image(PixelStorage storage, UnsignedInt format, UnsignedInt formatExtra, UnsignedInt pixelSize) noexcept;

        /** @overload
         *
         * Equivalent to the above for @p format already wrapped with
         * @ref pixelFormatWrap().
         */
        explicit Image(PixelStorage storage, PixelFormat format, UnsignedInt formatExtra, UnsignedInt pixelSize) noexcept;

        /**
         * @brief Construct an image with implementation-specific pixel format
         * @param storage           Storage of pixel data
         * @param format            Format of pixel data
         * @param formatExtra       Additional pixel format specifier
         * @param size              Image size
         * @param data              Image data
         *
         * Uses ADL to find a corresponding @cpp pixelSize(T, U) @ce overload,
         * then calls @ref Image(PixelStorage, UnsignedInt, UnsignedInt, UnsignedInt, const VectorTypeFor<dimensions, Int>&, Containers::Array<char>&&)
         * with calculated pixel size.
         */
        template<class T, class U> explicit Image(PixelStorage storage, T format, U formatExtra, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data) noexcept;

        /**
         * @brief Construct an image with implementation-specific pixel format
         * @param storage           Storage of pixel data
         * @param format            Format of pixel data
         * @param size              Image size
         * @param data              Image data
         *
         * Uses ADL to find a corresponding @cpp pixelSize(T) @ce overload,
         * then calls @ref Image(PixelStorage, UnsignedInt, UnsignedInt, UnsignedInt, const VectorTypeFor<dimensions, Int>&, Containers::Array<char>&&)
         * with calculated pixel size and @p formatExtra set to @cpp 0 @ce.
         */
        template<class T> explicit Image(PixelStorage storage, T format, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data) noexcept;

        /**
         * @brief Construct an image with implementation-specific pixel format
         * @param format            Format of pixel data
         * @param formatExtra       Additional pixel format specifier
         * @param size              Image size
         * @param data              Image data
         *
         * Equivalent to calling @ref Image(PixelStorage, T, U, const VectorTypeFor<dimensions, Int>&, Containers::Array<char>&&)
         * with default-constructed @ref PixelStorage.
         */
        template<class T, class U> explicit Image(T format, U formatExtra, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data) noexcept: Image{{}, format, formatExtra, size, std::move(data)} {}

        /**
         * @brief Construct an image with implementation-specific pixel format
         * @param format            Format of pixel data
         * @param size              Image size
         * @param data              Image data
         *
         * Equivalent to calling @ref Image(PixelStorage, T, const VectorTypeFor<dimensions, Int>&, Containers::Array<char>&&)
         * with default-constructed @ref PixelStorage.
         */
        template<class T> explicit Image(T format, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data) noexcept: Image{{}, format, size, std::move(data)} {}

        /**
         * @brief Construct an image placeholder with implementation-specific pixel format
         * @param storage           Storage of pixel data
         * @param format            Format of pixel data
         * @param formatExtra       Additional pixel format specifier
         *
         * Uses ADL to find a corresponding @cpp pixelSize(T, U) @ce overload,
         * then calls @ref Image(PixelStorage, UnsignedInt, UnsignedInt, UnsignedInt)
         * with calculated pixel size.
         */
        template<class T, class U> /*implicit*/ Image(PixelStorage storage, T format, U formatExtra) noexcept;

        /**
         * @brief Construct an image placeholder with implementation-specific pixel format
         * @param format            Format of pixel data
         * @param formatExtra       Additional pixel format specifier
         *
         * Equivalent to calling @ref Image(PixelStorage, T, U) with
         * default-constructed @ref PixelStorage.
         */
        template<class T, class U> /*implicit*/ Image(T format, U formatExtra) noexcept: Image{{}, format, formatExtra} {}

        /**
         * @brief Construct an image placeholder with implementation-specific pixel format
         * @param storage           Storage of pixel data
         * @param format            Format of pixel data
         *
         * Uses ADL to find a corresponding @cpp pixelSize(T) @ce overload,
         * then calls @ref Image(PixelStorage, UnsignedInt, UnsignedInt, UnsignedInt)
         * with calculated pixel size and @p formatExtra set to @cpp 0 @ce.
         */
        template<class T> /*implicit*/ Image(PixelStorage storage, T format) noexcept;

        /**
         * @brief Construct an image placeholder with implementation-specific pixel format
         * @param format            Format of pixel data
         *
         * Equivalent to calling @ref Image(PixelStorage, T) with
         * default-constructed @ref PixelStorage.
         */
        template<class T> /*implicit*/ Image(T format) noexcept: Image{{}, format} {}

        /** @brief Copying is not allowed */
        Image(const Image<dimensions>&) = delete;

        /** @brief Move constructor */
        Image(Image<dimensions>&& other) noexcept;

        /** @brief Copying is not allowed */
        Image<dimensions>& operator=(const Image<dimensions>&) = delete;

        /** @brief Move assignment */
        Image<dimensions>& operator=(Image<dimensions>&& other) noexcept;

        /** @brief Conversion to view */
        /* Not restricted to const&, because we might want to pass the view to
           another function in an oneliner (e.g. saving screenshot) */
        /*implicit*/ operator ImageView<dimensions>() const;

        /** @brief Storage of pixel data */
        PixelStorage storage() const { return _storage; }

        /**
         * @brief Format of pixel data
         *
         * Returns either a defined value from the @ref PixelFormat enum or a
         * wrapped implementation-specific value. Use
         * @ref isPixelFormatImplementationSpecific() to distinguish the case
         * and @ref pixelFormatUnwrap() to extract an implementation-specific
         * value, if needed.
         * @see @ref formatExtra()
         */
        PixelFormat format() const { return _format; }

        /**
         * @brief Additional pixel format specifier
         *
         * Some implementations (such as OpenGL) define a pixel format using
         * two values. This field contains the second implementation-specific
         * value verbatim, if any. See @ref format() for more information.
         */
        UnsignedInt formatExtra() const { return _formatExtra; }

        #if defined(MAGNUM_BUILD_DEPRECATED) && defined(MAGNUM_TARGET_GL)
        /**
         * @brief Data type of pixel data
         * @deprecated Cast @ref formatExtra() to @ref GL::PixelType instead.
         */
        CORRADE_DEPRECATED("cast formatExtra() to GL::PixelType instead") GL::PixelType type() const { return GL::PixelType(_formatExtra); }
        #endif

        /**
         * @brief Pixel size (in bytes)
         *
         * @see @ref pixelSize(PixelFormat)
         */
        UnsignedInt pixelSize() const { return _pixelSize; }

        /** @brief Image size */
        VectorTypeFor<dimensions, Int> size() const { return _size; }

        /**
         * @brief Image data properties
         *
         * See @ref PixelStorage::dataProperties() for more information.
         */
        std::pair<VectorTypeFor<dimensions, std::size_t>, VectorTypeFor<dimensions, std::size_t>> dataProperties() const {
            return Implementation::imageDataProperties<dimensions>(*this);
        }

        /**
         * @brief Raw data
         *
         * @see @ref release()
         */
        Containers::ArrayView<char> data() & { return _data; }
        Containers::ArrayView<char> data() && = delete; /**< @overload */

        /** @overload */
        Containers::ArrayView<const char> data() const & { return _data; }
        Containers::ArrayView<const char> data() const && = delete; /**< @overload */

        /** @overload */
        template<class T = char> T* data() {
            return reinterpret_cast<T*>(_data.data());
        }

        /** @overload */
        template<class T = char> const T* data() const {
            return reinterpret_cast<const T*>(_data.data());
        }

        #if defined(MAGNUM_BUILD_DEPRECATED) && defined(MAGNUM_TARGET_GL)
        /**
         * @brief Set image data
         * @param storage           Storage of pixel data
         * @param format            Format of pixel data
         * @param formatExtra       Additional pixel format specifier
         * @param size              Image size
         * @param data              Image data
         *
         * @deprecated Move-assign a new instance instead.
         *
         * Deletes previous data and replaces them with new. The @p data array
         * is expected to be of proper size for given parameters.
         * @see @ref release()
         */
        template<class T, class U> CORRADE_DEPRECATED("move-assign a new instance instead") void setData(PixelStorage storage, T format, U formatExtra, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data) {
            *this = Image<dimensions>{storage, format, formatExtra, size, std::move(data)};
        }

        /**
         * @brief Set image data
         * @param format            Format of pixel data
         * @param formatExtra       Additional pixel format specifier
         * @param size              Image size
         * @param data              Image data
         *
         * @deprecated Move-assign a new instance instead.
         *
         * Equivalent to calling @ref setData(PixelStorage, T, U, const VectorTypeFor<dimensions, Int>&, Containers::Array<char>&&)
         * with default-constructed @ref PixelStorage.
         */
        template<class T, class U> CORRADE_DEPRECATED("move-assign a new instance instead") void setData(T format, U formatExtra, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data) {
            *this = Image<dimensions>{format, formatExtra, size, std::move(data)};
        }
        #endif

        /**
         * @brief Release data storage
         *
         * Releases the ownership of the data array and resets @ref size() to
         * zero. The state afterwards is equivalent to moved-from state.
         * @see @ref data()
         */
        Containers::Array<char> release();

    private:
        PixelStorage _storage;
        PixelFormat _format;
        UnsignedInt _formatExtra;
        UnsignedInt _pixelSize;
        Math::Vector<Dimensions, Int> _size;
        Containers::Array<char> _data;
};

/** @brief One-dimensional image */
typedef Image<1> Image1D;

/** @brief Two-dimensional image */
typedef Image<2> Image2D;

/** @brief Three-dimensional image */
typedef Image<3> Image3D;

/**
@brief Compressed image

Stores multi-dimensional compressed image data together with layout and
compressed block format description. See @ref CompressedImageView for a
non-owning alternative.

This class can act as a drop-in replacement for @ref CompressedImageView and
@ref Trade::ImageData APIs and is implicitly convertible to
@ref CompressedImageView. Particular graphics API wrappers provide additional
image classes, for example @ref GL::CompressedBufferImage. See also @ref Image
for equivalent functionality targeted on non-compressed image formats.

@section CompressedImage-usage Basic usage

The image takes ownership of a passed @ref Corrade::Containers::Array, together
with storing image size and one of the generic @ref CompressedPixelFormat
values:

@snippet Magnum.cpp CompressedImage-usage

It's also possible to create just an image placeholder, storing only the image
properties without data or size. That is useful for example to specify desired
format of image queries in graphics APIs:

@snippet Magnum.cpp CompressedImage-usage-query

As with @ref CompressedImageView, this class supports extra storage parameters
and implementation-specific compressed pixel format specification. See its
documentation for more information.

@see @ref CompressedImage1D, @ref CompressedImage2D, @ref CompressedImage3D
*/
template<UnsignedInt dimensions> class CompressedImage {
    public:
        enum: UnsignedInt {
            Dimensions = dimensions /**< Image dimension count */
        };

        /**
         * @brief Constructor
         * @param storage           Storage of compressed pixel data
         * @param format            Format of compressed pixel data
         * @param size              Image size
         * @param data              Image data
         */
        explicit CompressedImage(CompressedPixelStorage storage, CompressedPixelFormat format, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data) noexcept;

        /**
         * @brief Constructor
         * @param format            Format of compressed pixel data
         * @param size              Image size
         * @param data              Image data
         *
         * Equivalent to calling @ref CompressedImage(CompressedPixelStorage, CompressedPixelFormat, const VectorTypeFor<dimensions, Int>&, Containers::Array<char>&&)
         * with default-constructed @ref CompressedPixelStorage.
         */
        explicit CompressedImage(CompressedPixelFormat format, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data) noexcept: CompressedImage{{}, format, size, std::move(data)} {}

        /**
         * @brief Construct a compressed image with implementation-specific format
         * @param storage           Storage of compressed pixel data
         * @param format            Format of compressed pixel data
         * @param size              Image size
         * @param data              Image data
         *
         * Uses @ref compressedPixelFormatWrap() internally to convert
         * @p format to @ref CompressedPixelFormat.
         */
        template<class T> explicit CompressedImage(CompressedPixelStorage storage, T format, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data) noexcept;

        /**
         * @brief Construct a compressed image with implementation-specific format
         * @param format            Format of compressed pixel data
         * @param size              Image size
         * @param data              Image data
         *
         * Equivalent to calling @ref CompressedImage(CompressedPixelStorage, T, const VectorTypeFor<dimensions, Int>&, Containers::Array<char>&&)
         * with default-constructed @ref CompressedPixelStorage.
         */
        template<class T> explicit CompressedImage(T format, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data) noexcept: CompressedImage{{}, format, size, std::move(data)} {}

        /**
         * @brief Construct an image placeholder
         * @param storage           Storage of compressed pixel data
         *
         * Format is undefined, size is zero and data is @cpp nullptr @ce. Move
         * over a non-empty instance to make it useful.
         */
        /*implicit*/ CompressedImage(CompressedPixelStorage storage) noexcept;

        /**
         * @brief Construct an image placeholder
         *
         * Equivalent to calling @ref CompressedImage(CompressedPixelStorage)
         * with default-constructed @ref CompressedPixelStorage.
         */
        /*implicit*/ CompressedImage() noexcept: CompressedImage{{}} {}

        /** @brief Copying is not allowed */
        CompressedImage(const CompressedImage<dimensions>&) = delete;

        /** @brief Move constructor */
        CompressedImage(CompressedImage<dimensions>&& other) noexcept;

        /** @brief Copying is not allowed */
        CompressedImage<dimensions>& operator=(const CompressedImage<dimensions>&) = delete;

        /** @brief Move assignment */
        CompressedImage<dimensions>& operator=(CompressedImage<dimensions>&& other) noexcept;

        /** @brief Conversion to view */
        /*implicit*/ operator CompressedImageView<dimensions>() const;

        /** @brief Storage of compressed pixel data */
        CompressedPixelStorage storage() const { return _storage; }

        /**
         * @brief Format of compressed pixel data
         *
         * Returns either a defined value from the @ref CompressedPixelFormat
         * enum or a wrapped implementation-specific value. Use
         * @ref isCompressedPixelFormatImplementationSpecific() to distinguish
         * the case and @ref compressedPixelFormatUnwrap() to extract an
         * implementation-specific value, if needed.
         */
        CompressedPixelFormat format() const { return _format; }

        /** @brief Image size */
        VectorTypeFor<dimensions, Int> size() const { return _size; }

        /**
         * @brief Compressed image data properties
         *
         * See @ref CompressedPixelStorage::dataProperties() for more
         * information.
         */
        std::pair<VectorTypeFor<dimensions, std::size_t>, VectorTypeFor<dimensions, std::size_t>> dataProperties() const {
            return Implementation::compressedImageDataProperties<dimensions>(*this);
        }

        /**
         * @brief Raw data
         *
         * @see @ref release()
         */
        Containers::ArrayView<char> data() { return _data; }

        /** @overload */
        Containers::ArrayView<const char> data() const { return _data; }

        /** @overload */
        template<class T> T* data() {
            return reinterpret_cast<T*>(_data.data());
        }

        /** @overload */
        template<class T> const T* data() const {
            return reinterpret_cast<const T*>(_data.data());
        }

        #if defined(MAGNUM_BUILD_DEPRECATED) && defined(MAGNUM_TARGET_GL)
        /**
         * @brief Set image data
         * @param storage           Storage of compressed pixel data
         * @param format            Format of compressed pixel data
         * @param size              Image size
         * @param data              Image data
         *
         * @deprecated Move-assign a new instance instead.
         *
         * Deletes previous data and replaces it with @p data.
         * @see @ref release()
         */
        CORRADE_DEPRECATED("move-assign a new instance instead") void setData(CompressedPixelStorage storage, CompressedPixelFormat format, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data) {
            *this = CompressedImage<dimensions>{storage, GL::CompressedPixelFormat(format), size, std::move(data)};
        }

        /**
         * @brief Set image data
         * @param format            Format of compressed pixel data
         * @param size              Image size
         * @param data              Image data
         *
         * @deprecated Move-assign a new instance instead.
         *
         * Equivalent to calling @ref setData(CompressedPixelStorage, CompressedPixelFormat, const VectorTypeFor<dimensions, Int>&, Containers::Array<char>&&)
         * with default-constructed @ref CompressedPixelStorage.
         */
        CORRADE_DEPRECATED("move-assign a new instance instead") void setData(CompressedPixelFormat format, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data) {
            *this = CompressedImage<dimensions>{GL::CompressedPixelFormat(format), size, std::move(data)};
        }
        #endif

        /**
         * @brief Release data storage
         *
         * Releases the ownership of the data array and resets @ref size() to
         * zero. The state afterwards is equivalent to moved-from state.
         * @see @ref data()
         */
        Containers::Array<char> release();

    private:
        /* To be made public once block size and block data size are stored
           together with the image */
        explicit CompressedImage(CompressedPixelStorage storage, UnsignedInt format, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data) noexcept;

        CompressedPixelStorage _storage;
        CompressedPixelFormat _format;
        Math::Vector<Dimensions, Int> _size;
        Containers::Array<char> _data;
};

/** @brief One-dimensional compressed image */
typedef CompressedImage<1> CompressedImage1D;

/** @brief Two-dimensional compressed image */
typedef CompressedImage<2> CompressedImage2D;

/** @brief Three-dimensional compressed image */
typedef CompressedImage<3> CompressedImage3D;

template<UnsignedInt dimensions> inline Image<dimensions>::Image(Image<dimensions>&& other) noexcept: _storage{std::move(other._storage)}, _format{std::move(other._format)}, _formatExtra{std::move(other._formatExtra)}, _pixelSize{std::move(other._pixelSize)}, _size{std::move(other._size)}, _data{std::move(other._data)} {
    other._size = {};
}

template<UnsignedInt dimensions> inline CompressedImage<dimensions>::CompressedImage(CompressedImage<dimensions>&& other) noexcept: _storage{std::move(other._storage)}, _format{std::move(other._format)}, _size{std::move(other._size)}, _data{std::move(other._data)}
{
    other._size = {};
}

template<UnsignedInt dimensions> inline Image<dimensions>& Image<dimensions>::operator=(Image<dimensions>&& other) noexcept {
    using std::swap;
    swap(_storage, other._storage);
    swap(_format, other._format);
    swap(_formatExtra, other._formatExtra);
    swap(_pixelSize, other._pixelSize);
    swap(_size, other._size);
    swap(_data, other._data);
    return *this;
}

template<UnsignedInt dimensions> inline CompressedImage<dimensions>& CompressedImage<dimensions>::operator=(CompressedImage<dimensions>&& other) noexcept {
    using std::swap;
    swap(_storage, other._storage);
    swap(_format, other._format);
    swap(_size, other._size);
    swap(_data, other._data);
    return *this;
}

template<UnsignedInt dimensions> inline Image<dimensions>::operator ImageView<dimensions>() const
{
    return ImageView<dimensions>{_storage, _format, _formatExtra, _pixelSize, _size, _data};
}

template<UnsignedInt dimensions> inline CompressedImage<dimensions>::operator CompressedImageView<dimensions>() const
{
    return CompressedImageView<dimensions>{_storage, _format, _size, _data};
}

template<UnsignedInt dimensions> inline Containers::Array<char> Image<dimensions>::release() {
    Containers::Array<char> data{std::move(_data)};
    _size = {};
    return data;
}

template<UnsignedInt dimensions> inline Containers::Array<char> CompressedImage<dimensions>::release() {
    Containers::Array<char> data{std::move(_data)};
    _size = {};
    return data;
}

template<UnsignedInt dimensions> template<class T, class U> inline Image<dimensions>::Image(const PixelStorage storage, const T format, const U formatExtra, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data) noexcept: Image{storage,
    #if defined(MAGNUM_BUILD_DEPRECATED) && defined(MAGNUM_TARGET_GL)
    Implementation::wrapPixelFormatIfNotGLSpecific(format),
    #else
    UnsignedInt(format),
    #endif
    UnsignedInt(formatExtra), Implementation::pixelSizeAdl(format, formatExtra), size, std::move(data)} {
    static_assert(sizeof(T) <= 4 && sizeof(U) <= 4,
        "format types larger than 32bits are not supported");
}

template<UnsignedInt dimensions> template<class T> inline  Image<dimensions>::Image(const PixelStorage storage, const T format, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data) noexcept: Image{storage, UnsignedInt(format), {}, Implementation::pixelSizeAdl(format), size, std::move(data)} {
    static_assert(sizeof(T) <= 4,
        "format types larger than 32bits are not supported");
}

template<UnsignedInt dimensions> template<class T, class U> inline Image<dimensions>::Image(const PixelStorage storage, const T format, const U formatExtra) noexcept: Image{storage,
    #if defined(MAGNUM_BUILD_DEPRECATED) && defined(MAGNUM_TARGET_GL)
    Implementation::wrapPixelFormatIfNotGLSpecific(format),
    #else
    UnsignedInt(format),
    #endif
    UnsignedInt(formatExtra), Implementation::pixelSizeAdl(format, formatExtra)} {
    static_assert(sizeof(T) <= 4 && sizeof(U) <= 4,
        "format types larger than 32bits are not supported");
}

template<UnsignedInt dimensions> template<class T> inline Image<dimensions>::Image(const PixelStorage storage, const T format) noexcept: Image{storage, UnsignedInt(format), {}, Implementation::pixelSizeAdl(format)} {
    static_assert(sizeof(T) <= 4,
        "format types larger than 32bits are not supported");
}

template<UnsignedInt dimensions> template<class T> inline CompressedImage<dimensions>::CompressedImage(const CompressedPixelStorage storage, const T format, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data) noexcept: CompressedImage{storage, UnsignedInt(format), size, std::move(data)} {
    static_assert(sizeof(T) <= 4,
        "format types larger than 32bits are not supported");
}

}

#endif
