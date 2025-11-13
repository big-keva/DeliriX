# include "../DOM-text.hpp"
# include "../DOM-dump.hpp"
# include "../DOM-load.hpp"
# include <moonycode/codes.h>
# include <mtc/serialize.h>
# include <mtc/test-it-easy.hpp>
# include <mtc/iStream.h>

template <> inline
auto  Serialize( std::string* to, const void* s, size_t len ) -> std::string*
  {  return to->append( (const char*)s, len ), to;  }

template <> inline
auto  Serialize( std::vector<char>* to, const void* p, size_t l ) -> std::vector<char>*
  {  return to->insert( to->end(), (const char*)p, (const char*)p + l ), to;  }

class ByteStreamOnString: public mtc::IByteStream
{
  std::string& str;

  implement_lifetime_stub

public:
  ByteStreamOnString( std::string& s ): str( s )
    {}
  uint32_t  Get(       void*, uint32_t ) override
    {  throw std::logic_error( "not implemented" );  }
  uint32_t  Put( const void* p, uint32_t l ) override
    {  str += std::string( (const char*)p, l );  return l;  }
  auto  ptr() const -> IByteStream*
    {  return (IByteStream*)this;  }
};

using namespace DeliriX;

const char  json[] =
  "[\n"
  "  \"aaa\",\n"
  "  \"bbb\",\n"
  "  { \"body\": [\n"
  "    \"first line in <body>\",\n"
  "    { \"p\": [\n"
  "      \"first line in <p> tag\",\n"
  "    ] },\n"
  "    \"third line in <body>\",\n"
  "  ] },\n"
  "  \"last text line \\\"with quotes\\\"\""
  "]\n";

const char  tags[] =
  "aaa\n"
  "bbb\n"
  "<body>\n"
  "  first line in &lt;body&gt;\n"
  "  second line in &lt;body&gt;\n"
  "<p>\n"
    "first line in &lt;p&gt; tag\n"
    "second line in &lt;p&gt; tag\n"
  "</p>\n"
  "third line in &lt;body&gt;\n"
  "<span>\n"
    "text line in &lt;span&gt;\n"
  "</span>\n"
  "fourth line in &lt;body&gt;\n"
  "</body>\n"
  "last text line \"with quotes\"";

TestItEasy::RegisterFunc  test_text( []()
{
  TEST_CASE( "texts/DOM" )
  {
    auto  dump = std::string();

    SECTION( "Text implements mini-DOM interface IText" )
    {
      auto  text = Text();

      SECTION( "it supports text blocks and block markups" )
      {
        SECTION( "text blocks are added to the end of image as strings or widestrings" )
        {
          if ( REQUIRE_NOTHROW( text.AddBlock( "aaa" ) ) )
            if ( REQUIRE( text.GetBlocks().size() == 1U ) )
              REQUIRE( text.GetBlocks().back().GetCharStr() == "aaa" );
          if ( REQUIRE_NOTHROW( text.AddBlock( "bbbb", 3 ) ) )
            if ( REQUIRE( text.GetBlocks().size() == 2U ) )
              REQUIRE( text.GetBlocks().back().GetCharStr() == "bbb" );
          if ( REQUIRE_NOTHROW( text.AddBlock( codepages::mbcstowide( codepages::codepage_utf8, "ccc" ) ) ) )
            if ( REQUIRE( text.GetBlocks().size() == 3U ) )
            {
              REQUIRE( text.GetBlocks().back().GetCharStr().empty() );

              if ( REQUIRE( !text.GetBlocks().back().GetWideStr().empty() ) )
                REQUIRE( text.GetBlocks().back().GetWideStr() == u"ccc" );
            }
        }
        SECTION( "text may be cleared" )
        {
          REQUIRE_NOTHROW( text.clear() );
          REQUIRE( text.GetBlocks().empty() );
        }
        SECTION( "tags cover lines and are closed automatcally" )
        {
          if ( REQUIRE_NOTHROW( text.AddBlock( "aaa" ) ) )
          {
            auto  tag = mtc::api<IText>();

            if ( REQUIRE_NOTHROW( tag = text.AddMarkupTag( "block" ) )
              && REQUIRE_NOTHROW( tag->AddBlock( "bbb" ) )
              && REQUIRE_NOTHROW( text.AddBlock( "ccc" ) ) )
            {
              if ( REQUIRE( text.GetMarkup().size() == 1U ) )
              {
                REQUIRE( text.GetMarkup().back().tagKey == "block" );
                REQUIRE( text.GetMarkup().back().uLower == 3 );
                REQUIRE( text.GetMarkup().back().uUpper == 5 );
              }
            }
          }
        }
      }
      /*
      SECTION( "it may be created with alternate allocator" )
      {
        auto  arena = mtc::Arena();
        auto  image = arena.Create<BaseDocument<mtc::Arena::allocator<char>>>();

        SECTION( "elements are allocated in arena with no need to deallocate" )
        {
          image->AddString( "This is a test string object to be allocated in memory arena" );
          image->AddString( codepages::mbcstowide( codepages::codepage_utf8,
            "This is a test widestring to be allocated in memory arena" ) );
        }
      }
      */
    }
    SECTION( "Text may be serialized" )
    {
      auto  text = Text();

      text.AddBlock( "aaa" );
        text.AddMarkupTag( "bbb" )->AddBlock( "bbb" );
      text.AddBlock( "ccc" );

      SECTION( "* as json" )
      {
        if ( REQUIRE_NOTHROW( text.Serialize( dump_as::Json( dump_as::MakeOutput( &dump ) ).ptr() ) ) )
        {
          REQUIRE( dump ==
            "[\n"
            "  \"aaa\",\n"""
            "  { \"bbb\": [\n"
            "    \"bbb\"\n"
            "  ] },\n"
            "  \"ccc\"\n"
            "]" );
        }
      }
      SECTION( "* as tags" )
      {
        dump.clear();

        if ( REQUIRE_NOTHROW( text.Serialize( dump_as::Tags( dump_as::MakeOutput( &dump ) ).ptr() ) ) )
        {
          REQUIRE( dump ==
            "aaa\n"
            "<bbb>\n"
            "  bbb\n"
            "</bbb>\n"
            "ccc\n" );
        }
      }
      SECTION( "* as dump" )
      {
        dump.clear();

        REQUIRE_NOTHROW( text.Serialize( &dump ) );
        REQUIRE( dump.length() == text.GetBufLen() );
      }
      SECTION( "Text keeps the tags order for all the tags including empty" )
      {
        auto  tags = std::string();

        text.clear();

        auto  tb = text.AddMarkupTag( "table" );
        auto  tr = tb->AddMarkupTag( "tr" );
        auto  td = tr->AddMarkupTag( "td" );
                   td->AddBlock( "s1" );
              td = tr->AddMarkupTag( "td" );

        if ( REQUIRE_NOTHROW( text.Serialize( dump_as::Tags( dump_as::MakeOutput( &tags ) ).ptr() ) ) )
        {
          REQUIRE( tags ==
            "<table>\n"
            "  <tr>\n"
            "    <td>\n"
            "      s1\n"
            "    </td>\n"
            "  </tr>\n"
            "</table>\n" );
        }
      }
    }
    SECTION( "Text may be loaded from source" )
    {
      auto  text = Text();

      SECTION( "* as json" )
      {
        REQUIRE_NOTHROW( load_as::Json( &text, load_as::MakeSource( json ) ) );

        REQUIRE( text.GetBlocks().size() == 6U );
        REQUIRE( text.GetMarkup().size() == 2U );
      }

      SECTION( "* as tags" )
      {
        text.clear();
        REQUIRE_NOTHROW( load_as::Tags( &text, load_as::MakeSource( tags ) ) );

        REQUIRE( text.GetBlocks().size() == 10U );
        REQUIRE( text.GetMarkup().size() == 3U );
      }

      SECTION( "* as dump" )
      {
        text.clear();
        REQUIRE_NOTHROW( text.FetchFrom( mtc::sourcebuf( dump ).ptr() ) );

        REQUIRE( text.GetBlocks().size() == 3U );
        REQUIRE( text.GetMarkup().size() == 1U );
      }
    }
    SECTION( "Text may be initialized with initializer list" )
    {
      auto  text = Text{
        "this is a first text string",
        { "tag", {
            "first tag string",
            "second tag string" } },
        "this is a second text string" };
      auto  outs = std::string();

      REQUIRE_NOTHROW( text.Serialize( dump_as::Json(
        dump_as::MakeOutput( &outs ) ) ) );
      REQUIRE( outs == "[\n"
        "  \"this is a first text string\",\n"
        "  { \"tag\": [\n"
        "    \"first tag string\",\n"
        "    \"second tag string\"\n"
        "  ] },\n"
        "  \"this is a second text string\"\n"
        "]" );
    }
  }
  TEST_CASE( "texts/word-break" )
  {
    auto  inText = Text{
      "Первая строка текста: просто строка",
      "Вторая строка в новом абзаце",
      { "tag-1", {
        "Строка внутри тега",
        { "tag-2", {
          "Строка внутри вложенного тега" } } } },
      "Третья строка."
    };
    auto  ucText = Text();

    SECTION( "any text may be converted to utf16" )
    {
      REQUIRE_NOTHROW( CopyUtf16( &ucText, inText ) );

      REQUIRE( ucText.GetMarkup().size() == inText.GetMarkup().size() );
      REQUIRE( ucText.GetBlocks().size() == inText.GetBlocks().size() );

      for ( auto& next: ucText.GetBlocks() )
        REQUIRE( next.GetEncoding() == uint32_t(-1) );
    }
  }
  /*
  TEST_CASE( "text/pack-unpack" )
  {
    SECTION( "array of TextToken may be packed" )
    {
      auto  ucText = Document();
      auto  txBody = BaseBody<std::allocator<char>>();

      Document{
        "Первая строка текста: просто строка",
        "Вторая строка в новом абзаце",
        { "tag-1", {
          "Строка внутри тега",
          { "tag-2", {
            "Строка внутри вложенного тега" } } } },
        "Третья строка." }.CopyUtf16( &ucText );
      BreakWords( txBody, ucText.GetBlocks() );

      auto  as_vec = std::vector<char>();
      auto  as_str = std::string();
      auto  as_stm = std::string();

      SECTION( "* as std::vector<char>" )
      {
        REQUIRE_NOTHROW( as_vec = PackWords( txBody.GetTokens() ) );
      }
      SECTION( "* as std::string through function<>" )
      {
        REQUIRE_NOTHROW( PackWords( [&]( const void* p, size_t l )
          {  as_str += std::string( (const char*)p, l );  }, txBody.GetTokens() ) );
      }
      SECTION( "* as IByteStream" )
      {
        REQUIRE_NOTHROW( PackWords( ByteStreamOnString( as_stm ).ptr(), txBody.GetTokens() ) );
      }
      SECTION( "and the results are the same" )
      {
        REQUIRE( as_str == as_stm );
        REQUIRE( as_stm.length() == as_vec.size() );
        REQUIRE( memcmp( as_stm.data(), as_vec.data(), as_stm.size() ) == 0 );
      }

      SECTION( "the packed words may be unpacked" )
      {
        auto  unpack = Body();

        if ( REQUIRE_NOTHROW( unpack = UnpackWords( as_vec ) ) )
          REQUIRE( unpack.GetTokens() == txBody.GetTokens() );
      }
    }
  }
  */
} );
