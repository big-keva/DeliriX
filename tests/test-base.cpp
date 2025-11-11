# include "../archive.hpp"
# include "mock-buff.hpp"
# include <mtc/byteBuffer.h>
# include <mtc/test-it-easy.hpp>

using namespace DeliriX;

extern unsigned char  sample_zipzip_buf[];
extern unsigned       sample_zipzip_len;

TestItEasy::RegisterFunc  test_text_base( []()
{
  TEST_CASE( "DeliriX/internals" )
  {
    SECTION( "it supports reading zip archives" )
    {
      mtc::api<IArchive>  archive;

      SECTION( "for empty source or invalid data, OpenZip() throws invalid_argument" )
      {
        REQUIRE_EXCEPTION( OpenZip( {} ), std::invalid_argument );
        REQUIRE_EXCEPTION( OpenZip( mtc::CreateByteBuffer( "non-zip archive", 15 ).ptr() ), std::invalid_argument );
      }
      SECTION( "zip archive files are opened" )
      {
        if ( REQUIRE_NOTHROW( archive = OpenZip( mtc::CreateByteBuffer( sample_zipzip_buf, sample_zipzip_len ).ptr() ) )
          && REQUIRE( archive != nullptr ) )
        {
          SECTION( "objects in archive may be accessed as buffers" )
          {
            auto  buffer = mtc::api<const mtc::IByteBuffer>();

            SECTION( "getting non-existing objects returns nullptr" )
            {
              if ( REQUIRE_NOTHROW( buffer = archive->GetObject( "non-existing-object" ) ) )
                REQUIRE( buffer == nullptr );
            }
            SECTION( "getting existing objects return those objects" )
            {
              if ( REQUIRE_NOTHROW( buffer = archive->GetObject( "zip_data.txt" ) )
                && REQUIRE( buffer != nullptr ) )
              {
                if ( REQUIRE( buffer->GetLen() == 28 ) )
                  REQUIRE( memcmp( buffer->GetPtr(), "this is a test zip file data", 28 ) == 0 );
              }
            }
          }
        }
      }
    }
  }
} );
