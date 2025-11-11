# if !defined( __DeliriX_xml_hpp__ )
# define __DeliriX_xml_hpp__
# include "../text-API.hpp"
# include <mtc/iBuffer.h>

namespace DeliriX::tinyxml
{

  class Error: public std::runtime_error
    {  using std::runtime_error::runtime_error;  };

  void  ParseXML( IText*, const mtc::api<const mtc::IByteBuffer>& );

}

# endif   // !__DeliriX_xml_hpp__
