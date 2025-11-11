# include "../formats.hpp"
# include "../archive.hpp"
# include "../DOM-text.hpp"
# include "../DOM-dump.hpp"
# include "mock-buff.hpp"
# include <mtc/test-it-easy.hpp>
# include <mtc/wcsstr.h>
# include <mtc/zmap.h>

using namespace DeliriX;

extern unsigned char  sample_docxDeliriX_buf[];
extern unsigned       sample_docxDeliriX_len;

TestItEasy::RegisterFunc  test_docx( []()
{
  TEST_CASE( "DeliriX/docx" )
  {
    SECTION( "DOCX parser loads .docx documents" )
    {
      SECTION( "with empty buffer or output, it throws std::invalid_argument" )
      {
        Text  text;

        REQUIRE_EXCEPTION( ParseDOCX( nullptr, mtc::CreateByteBuffer( "aaa", 3 ).ptr() ),
          std::invalid_argument );
        REQUIRE_EXCEPTION( ParseDOCX( &text, nullptr ),
          std::invalid_argument );
      }
      SECTION( "with correct data, it parses xml" )
      {
        Text  text;

        if ( REQUIRE_NOTHROW( ParseDOCX( &text, mtc::CreateByteBuffer( sample_docxDeliriX_buf, sample_docxDeliriX_len ).ptr() )))
        {
          text.Serialize( dump_as::Tags( dump_as::MakeOutput( stdout ) ) );
        }
      }
    }
  }
} );
