#ifndef Magnum_AbstractResourceLoader_h
#define Magnum_AbstractResourceLoader_h
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
 * @brief Class @ref Magnum::AbstractResourceLoader
 */

#include <string>

#include "Magnum/ResourceManager.h"

namespace Magnum {

/**
@brief Base for resource loaders

Provides (a)synchronous resource loading for @ref ResourceManager.

@section AbstractResourceLoader-usage Usage and subclassing

Usage is done by subclassing. Subclass instances can be added to
@ref ResourceManager using @ref ResourceManager::setLoader(). After adding the
loader, each call to @ref ResourceManager::get() will call @ref load()
implementation unless the resource is already loaded (or loading is in
progress). Note that resources requested before the loader was added are not
affected by the loader.

Subclassing is done by implementing at least @ref doLoad() function. The
loading can be done synchronously or asynchronously (i.e., in another thread).
The base implementation provides interface to @ref ResourceManager and manages
loading progress (which is then available through functions @ref requestedCount(),
@ref loadedCount() and @ref notFoundCount()). You shouldn't access the
@ref ResourceManager directly when loading the data.

In your @ref doLoad() implementation, after your resources are loaded, call
@ref set() to pass them to @ref ResourceManager or call @ref setNotFound() to
indicate that the resource was not found.

You can also implement @ref doName() to provide meaningful names for resource
keys.

Example implementation for synchronous mesh loader:

@code{.cpp}
class MeshResourceLoader: public AbstractResourceLoader<Mesh> {
    void doLoad(ResourceKey key) override {
        // Load the mesh...

        // Not found
        if(!found) {
            setNotFound(key);
            return;
        }

        // Found, pass it to resource manager
        set(key, mesh, state, policy);
    }
};
@endcode

You can then add it to resource manager instance like this. Note that the
manager automatically deletes the all loaders on destruction before unloading
all resources. It allows you to use resources in the loader itself without
having to delete the loader explicitly to ensure proper resource unloading. In
the following code, however, the loader destroys itself (and removes itself
from the manager) before the manager is destroyed.

@code{.cpp}
MyResourceManager manager;
MeshResourceLoader loader;

manager->setLoader(&loader);

// This will now automatically request the mesh from loader by calling load()
Resource<Mesh> myMesh = manager->get<Mesh>("my-mesh");
@endcode

@todoc How about working with resources of different data types (i.e. mesh
    buffers), should that be allowed?
*/
template<class T> class AbstractResourceLoader {
    public:
        explicit AbstractResourceLoader(): manager(nullptr), _requestedCount(0), _loadedCount(0), _notFoundCount(0) {}

        virtual ~AbstractResourceLoader();

        /**
         * @brief Count of requested resources
         *
         * Count of resources requested by calling @ref load().
         */
        std::size_t requestedCount() const { return _requestedCount; }

        /**
         * @brief Count of not found resources
         *
         * Count of resources requested by calling @ref load(), but not found
         * by the loader.
         */
        std::size_t notFoundCount() const { return _notFoundCount; }

        /**
         * @brief Count of loaded resources
         *
         * Count of resources requested by calling @ref load(), but not found
         * by the loader.
         */
        std::size_t loadedCount() const { return _loadedCount; }

        /**
         * @brief Resource name corresponding to given key
         *
         * If no such resource exists or the resource name is not available,
         * returns empty string.
         */
        std::string name(ResourceKey key) const { return doName(key); }

        /**
         * @brief Request resource to be loaded
         *
         * If the resource isn't yet loaded or loading, state of the resource
         * is set to @ref ResourceState::Loading and count of requested
         * features is incremented. Depending on implementation the resource
         * might be loaded synchronously or asynchronously.
         *
         * @see @ref ResourceManager::state(), @ref requestedCount(),
         *      @ref notFoundCount(), @ref loadedCount()
         */
        void load(ResourceKey key);

    protected:
        /**
         * @brief Set loaded resource to resource manager
         *
         * Also increments count of loaded resources. Parameter @p state must
         * be either @ref ResourceDataState::Mutable or
         * @ref ResourceDataState::Final. See @ref ResourceManager::set() for
         * more information.
         * @see @ref loadedCount()
         */
        void set(ResourceKey key, T* data, ResourceDataState state, ResourcePolicy policy);

        /** @overload */
        template<class U> void set(ResourceKey key, U&& data, ResourceDataState state, ResourcePolicy policy) {
            set(key, new typename std::decay<U>::type(std::forward<U>(data)), state, policy);
        }

        /**
         * @brief Set loaded resource to resource manager
         *
         * Same as above function with state set to @ref ResourceDataState::Final
         * and policy to @ref ResourcePolicy::Resident.
         */
        void set(ResourceKey key, T* data) {
            set(key, data, ResourceDataState::Final, ResourcePolicy::Resident);
        }

        /** @overload */
        template<class U> void set(ResourceKey key, U&& data) {
            set(key, new typename std::decay<U>::type(std::forward<U>(data)));
        }

        /**
         * @brief Mark resource as not found
         *
         * Also increments count of not found resources. See also
         * @ref ResourceManager::set() for more information.
         * @see @ref notFoundCount()
         */
        void setNotFound(ResourceKey key);

    #ifndef DOXYGEN_GENERATING_OUTPUT
    private:
    #else
    protected:
    #endif
        /**
         * @brief Implementation for @ref name()
         *
         * Default implementation returns empty string.
         */
        virtual std::string doName(ResourceKey key) const;

        /**
         * @brief Implementation for @ref load()
         *
         * See class documentation for reimplementation guide.
         */
        virtual void doLoad(ResourceKey key) = 0;

    private:
        #ifndef DOXYGEN_GENERATING_OUTPUT /* https://bugzilla.gnome.org/show_bug.cgi?id=776986 */
        friend Implementation::ResourceManagerData<T>;
        #endif

        Implementation::ResourceManagerData<T>* manager;
        std::size_t _requestedCount,
            _loadedCount,
            _notFoundCount;
};

template<class T> AbstractResourceLoader<T>::~AbstractResourceLoader() {
    if(manager) manager->_loader = nullptr;
}

template<class T> std::string AbstractResourceLoader<T>::doName(ResourceKey) const { return {}; }

template<class T> void AbstractResourceLoader<T>::load(ResourceKey key) {
    ++_requestedCount;
    /** @todo What policy for loading resources? */
    manager->set(key, nullptr, ResourceDataState::Loading, ResourcePolicy::Resident);

    doLoad(key);
}

template<class T> void AbstractResourceLoader<T>::set(ResourceKey key, T* data, ResourceDataState state, ResourcePolicy policy) {
    CORRADE_ASSERT(state == ResourceDataState::Mutable || state == ResourceDataState::Final,
        "AbstractResourceLoader::set(): state must be either Mutable or Final", );
    ++_loadedCount;
    manager->set(key, data, state, policy);
}

template<class T> inline void AbstractResourceLoader<T>::setNotFound(ResourceKey key) {
    ++_notFoundCount;
    /** @todo What policy for notfound resources? */
    manager->set(key, nullptr, ResourceDataState::NotFound, ResourcePolicy::Resident);
}

}

#endif
