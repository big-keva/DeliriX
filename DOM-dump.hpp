# if !defined( __DeliriX_DOM_dump_hpp__ )
# define __DeliriX_DOM_dump_hpp__
# include "text-API.hpp"
# include <moonycode/codes.h>
# include <mtc/serialize.h>
# include <functional>
# include <stdexcept>
# include <memory>

namespace DeliriX {
namespace dump_as {

  using SerializeFn = std::function<void(const char*, size_t)>;

  auto  Tags( SerializeFn, unsigned encode = codepages::codepage_utf8 ) -> mtc::api<IText>;
  auto  Json( SerializeFn ) -> mtc::api<IText>;

  template <class O>
  auto  MakeOutput( O* o ) -> SerializeFn
  {
    return [output = std::make_shared<O*>( o )]( const char* str, size_t len ) mutable
      {
        if ( (*output = ::Serialize( *output, str, len )) == nullptr )
          throw std::runtime_error( "serialization failed" );
      };
  }

}}

# endif // !__DeliriX_DOM_dump_hpp__
