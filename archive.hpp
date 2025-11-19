# if !defined( __DeliriX_archive_hpp__ )
# define __DeliriX_archive_hpp__
#include <string>
# include <mtc/byteBuffer.h>

namespace DeliriX
{

  struct IArchive: mtc::Iface
  {
    struct IEntry;

    virtual auto  GetFile( const char* ) -> mtc::api<const mtc::IByteBuffer> = 0;
    virtual auto  ReadDir() -> mtc::api<IEntry> = 0;
  };

  struct IArchive::IEntry: mtc::Iface
  {
    virtual auto  GetAttr() const -> uint32_t = 0;
    virtual auto  GetName() const -> std::string = 0;
    virtual auto  GetFile() const -> mtc::api<const mtc::IByteBuffer> = 0;
  };

  auto  OpenZip( const mtc::api<const mtc::IByteBuffer>& ) -> mtc::api<IArchive>;

}

# endif   // !__DeliriX_archive_hpp__
