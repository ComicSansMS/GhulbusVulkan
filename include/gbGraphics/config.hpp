#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_GBGRAPHICS_CONFIG_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_GBGRAPHICS_CONFIG_HPP

/** @file
 *
 * @brief General configuration for Graphics.
 * @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
 */

#include <gbGraphics/gbGraphics_Export.hpp>

/** Specifies the API for a function declaration.
 * When building as a dynamic library, this is used to mark the functions that will be exported by the library.
 */
#define GHULBUS_GRAPHICS_API GHULBUS_LIBRARY_GBGRAPHICS_EXPORT

/** \namespace GhulbusGraphics Namespace for the GhulbusGraphics library.
 * The implementation internally always uses this macro `GHULBUS_GRAPHICS_NAMESPACE` to refer to the namespace.
 * When building GhulbusGraphics yourself, you can globally redefine this macro to move to a different namespace.
 */
#ifndef GHULBUS_GRAPHICS_NAMESPACE
#   define GHULBUS_GRAPHICS_NAMESPACE GhulbusGraphics
#endif

#endif
