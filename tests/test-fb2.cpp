# include "../formats.hpp"
# include "../archive.hpp"
# include "../DOM-text.hpp"
# include "../DOM-dump.hpp"
# include "mock-buff.hpp"
# include <mtc/test-it-easy.hpp>
# include <mtc/wcsstr.h>
# include <mtc/zmap.h>

using namespace DeliriX;

extern unsigned char  sample_fb2Panov_buf[];
extern unsigned       sample_fb2Panov_len;

TestItEasy::RegisterFunc  test_fb2( []()
{
  TEST_CASE( "DeliriX/fb2" )
  {
    SECTION( "FB2 parser loads .fb2 documents" )
    {
      SECTION( "with empty buffer or output, it throws std::invalid_argument" )
      {
        Text  text;

        REQUIRE_EXCEPTION( ParseFB2( nullptr, mtc::CreateByteBuffer( "aaa", 3 ).ptr() ),
          std::invalid_argument );
        REQUIRE_EXCEPTION( ParseFB2( &text, nullptr ),
          std::invalid_argument );
      }
      SECTION( "with correct data, it parses xml" )
      {
        Text  text;

        if ( REQUIRE_NOTHROW( ParseFB2( &text, mtc::CreateByteBuffer( sample_fb2Panov_buf, sample_fb2Panov_len ).ptr() )))
        {
//          text.Serialize( dump_as::Tags( dump_as::MakeOutput( stdout ) ) );
        }
      }
    }
  }
} );
