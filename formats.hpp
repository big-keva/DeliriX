# if !defined( __DeliriX_formats_hpp__ )
# define __DeliriX_formats_hpp__
# include "text-API.hpp"
# include <mtc/iBuffer.h>

namespace DeliriX
{

  int   ParseODT  ( IText*, const mtc::api<const mtc::IByteBuffer>& );
  int   ParseDOCX ( IText*, const mtc::api<const mtc::IByteBuffer>& );
  int   ParseFB2  ( IText*, const mtc::api<const mtc::IByteBuffer>& );

}

# endif // !__DeliriX_formats_hpp__
