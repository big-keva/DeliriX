# include "../archive.hpp"
# include <minizip/unzip.h>
# include <stdexcept>
# include <cstring>
# include <vector>

namespace DeliriX {

  class ZipArchive final: public IArchive
  {
    mtc::api<const mtc::IByteBuffer>  buffer;
    unzFile                           zipped;

  public:
    ZipArchive( const mtc::api<const mtc::IByteBuffer>&, unzFile );
   ~ZipArchive();

    auto  GetObject( const char* path ) -> mtc::api<mtc::IByteBuffer> override;

    implement_lifetime_control
  };

  class ByteBuff final: public std::vector<char>, public mtc::IByteBuffer
  {
    implement_lifetime_control

  public:
    const char* GetPtr() const override {  return data();  }
    size_t      GetLen() const override {  return size();  }
    int         SetBuf( const void*, size_t ) override  {  return -1;  }
    int         SetLen( size_t ) override {  return -1;  }
  };

  extern zlib_filefunc_def_s zlib_funcs;

  // ZipArchive implementation

  ZipArchive::ZipArchive( const mtc::api<const mtc::IByteBuffer>& buf, unzFile zip ):
    buffer( buf ),
    zipped( zip )
  {
  }

  ZipArchive::~ZipArchive()
  {
    if ( zipped != nullptr )
      unzClose( zipped );
  }

  auto  ZipArchive::GetObject( const char* objectname ) -> mtc::api<mtc::IByteBuffer>
  {
    if ( zipped != nullptr && unzLocateFile( zipped, objectname, 1 ) == UNZ_OK && unzOpenCurrentFile( zipped ) == UNZ_OK )
    {
      auto  zipbuf = mtc::api( new ByteBuff );
      char  buffer[0x400];
      long  cbread;

      while ( (cbread = unzReadCurrentFile( zipped, buffer, sizeof(buffer) )) > 0 )
        zipbuf->insert( zipbuf->end(), buffer, buffer + cbread );

      unzCloseCurrentFile( zipped );

      return zipbuf.ptr();
    }
    return nullptr;
  }

  auto  OpenZip( const mtc::api<const mtc::IByteBuffer>& src ) -> mtc::api<IArchive>
  {
    unzFile zip;

    if ( src == nullptr )
      throw std::invalid_argument( "archive::OpenZip source buffer is empty" );

    if ( (zip = unzOpen2( (const char*)src.ptr(), &zlib_funcs ) ) == nullptr )
      throw std::invalid_argument( "archive::OpenZip unzOpen2() failed" );

    return new ZipArchive( src, zip );
  }

  // zip interface functions

  struct STM
  {
    mtc::api<const mtc::IByteBuffer>  data;
    int64_t                           cpos = 0;
    int                               nerr = 0;
  };

  static  void* zip_open_file( void*, const char* name, int )
  {
    return new STM{ mtc::api<const mtc::IByteBuffer>( (mtc::IByteBuffer*)name ) };
  }

  static  uLong zip_read_file( void*, void* stm, void* ptr, uLong len )
  {
    auto  mem = static_cast<STM*>( stm );
    auto  cbr = std::min( mem->data->GetLen() - mem->cpos, len );

    if ( cbr > 0 )
      memcpy( ptr, mem->data->GetPtr() + mem->cpos, cbr );
    return mem->nerr = 0, mem->cpos += cbr, cbr;
  }

  static  uLong zip_write_file( void*, void* stm, const void*, uLong )
  {
    auto  mem = static_cast<STM*>( stm );

    return mem->nerr = EINVAL, -1;
  }

  static  long  zip_tell_file( void*, void* stm )
  {
    auto  mem = static_cast<STM*>( stm );

    return mem->nerr = 0, mem->cpos;
  }

  static  long  zip_seek_file( void*, void* stm, uLong pos32, int origin )
  {
    auto  mem = static_cast<STM*>( stm );

    switch ( origin )
    {
      case SEEK_SET:
        if ( pos32 > mem->data->GetLen() )
          return mem->nerr = ERANGE, -1;
        return mem->cpos = pos32, 0;
      case SEEK_CUR:
        return 0;
      case SEEK_END:
        return mem->cpos = mem->data->GetLen(), 0;
      default:
        return mem->nerr = EINVAL, -1;
    }
  }

  static  int   zip_close_file( void*, void* stm )
  {
    return delete (STM*)stm, 0;
  }

  static  int   zip_error_file( void*, void* stm )
  {
    auto  mem = static_cast<STM*>( stm );
    auto  err = mem->nerr;
    return mem->nerr = 0, err;
  }

  zlib_filefunc_def_s zlib_funcs{
    zip_open_file,
    zip_read_file,
    zip_write_file,
    zip_tell_file,
    zip_seek_file,
    zip_close_file,
    zip_error_file, nullptr
  };

}
