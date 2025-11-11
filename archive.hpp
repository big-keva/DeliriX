# if !defined( __DeliriX_archive_hpp__ )
# define __DeliriX_archive_hpp__
# include <mtc/byteBuffer.h>

namespace DeliriX
{

  struct IArchive: mtc::Iface
  {
    virtual auto  GetObject( const char* ) -> mtc::api<mtc::IByteBuffer> = 0;
  };

  auto  OpenZip( const mtc::api<const mtc::IByteBuffer>& ) -> mtc::api<IArchive>;

}

# endif   // !__DeliriX_archive_hpp__
