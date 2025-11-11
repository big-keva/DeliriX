# if !defined( __DeliriX_DOM_load_hpp__ )
# define __DeliriX_DOM_load_hpp__
# include "text-API.hpp"
# include <mtc/serialize.h>
# include <functional>
# include <stdexcept>
# include <memory>

namespace DeliriX {
namespace load_as {

  class ParseError : public std::runtime_error
    {  using std::runtime_error::runtime_error;  };

  void  Tags( mtc::api<IText>, std::function<char()> );
  void  Json( mtc::api<IText>, std::function<char()> );

  template <class S>
  auto  MakeSource( S* s ) -> std::function<char()>
  {
    return [source = std::make_shared<S*>( s )]() mutable -> char
      {
        char  c;

        if ( (*source = ::FetchFrom( *source, c )) == nullptr )
          throw std::runtime_error( "::FetchFrom() failed" );
        return c;
      };
  }

}}

# endif // !__DeliriX_DOM_load_hpp__
