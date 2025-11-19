# if !defined( __DeliriX_formats_hpp__ )
# define __DeliriX_formats_hpp__
# include "text-API.hpp"
# include <mtc/iBuffer.h>

namespace DeliriX
{

  class Error: public std::runtime_error
    {  using std::runtime_error::runtime_error;  };

  int   ParseXML  ( IText*, const mtc::api<const mtc::IByteBuffer>& );
  int   ParseODT  ( IText*, const mtc::api<const mtc::IByteBuffer>& );
  int   ParseDOCX ( IText*, const mtc::api<const mtc::IByteBuffer>& );
  int   ParseFB2  ( IText*, const mtc::api<const mtc::IByteBuffer>& );

}

# endif   // !__DeliriX_formats_hpp__
