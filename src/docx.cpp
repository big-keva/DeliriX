
# include "../archive.hpp"
# include "../formats.hpp"
# include <moonycode/codes.h>
# include <mtc/wcsstr.h>

namespace DeliriX
{

  class DOCX final: public IText
  {
    mtc::api<IText>   output;

    std::atomic_long  refCnt;

    mtc::widestr      string;

  public:
    DOCX( IText* tx ): output( tx ), refCnt( 0 ) {}
    DOCX( IText* tx, int inplace ): output( tx ), refCnt( inplace ) {}

    auto  AddMarkupTag( const std::string_view&, const markup_attribute& ) -> mtc::api<IText>  override;
    auto  AddParagraph( const Paragraph& ) -> Paragraph override;

    long  Attach() override;
    long  Detach() override;

  };

  class Para final: public IText
  {
    mtc::api<IText>   output;
    mtc::charstr      tagStr = "p";
    mtc::widestr      string;

  public:
    Para( IText* tx ): output( tx ) {}
   ~Para();

    auto  AddMarkupTag( const std::string_view&, const markup_attribute& ) -> mtc::api<IText>  override;
    auto  AddParagraph( const Paragraph& ) -> Paragraph override;

    implement_lifetime_control

  };

  // DOCX implementation

  auto  DOCX::AddMarkupTag( const std::string_view& tag, const markup_attribute& att ) -> mtc::api<IText>
  {
    static const std::initializer_list<const char*> ignoreTags = {
      "w:document",
      "w:body",
      "w:r",
      "w:t" };

    static const std::initializer_list<std::pair<const char*, const char*>> renameTags = {
      { "w:tbl", "table" },
      { "w:tr", "tr" },
      { "w:tc", "td" } };

    // check ignore tags
    for ( auto ignore : ignoreTags )
      if ( tag == ignore )  return this;

    // check renamed tags
    for ( auto& rename : renameTags )
      if ( tag == rename.first )  return new DOCX( output->AddMarkupTag( rename.second, att ) );

    if ( tag == "w:p" )
      return new Para( output );

  // headings
    if ( tag == "text:h" )
    {
      auto  outLevel = att.find( "text:outline-level" );
      auto  tagValue = std::string( "h" );

      if ( outLevel != att.end() )
        tagValue += outLevel->second;

      return new DOCX( output->AddMarkupTag( tagValue, {} ) );
    }

  // spaces
    if ( tag == "text:s" )
    {
      auto  getCount = att.find( "text:c" );
      auto  strCount = getCount != att.end() ? getCount->second : "1";
      auto  intCount = strtol( strCount.c_str(), nullptr, 10 );
      auto  strPaste = mtc::strprintf( mtc::strprintf( "%%%dc", std::max( intCount, 1L ) ).c_str(), ' ' );

      return IText::AddBlock( strPaste ), this;
    }

    return new DOCX( output->AddMarkupTag( tag, att ) );
  }

  auto  DOCX::AddParagraph( const Paragraph& para ) -> Paragraph
  {
    uint32_t  coding = para.GetEncoding();

    if ( coding == uint32_t(-1) ) string += para.GetWideStr();
      else  string += codepages::mbcstowide( coding, para.GetCharStr() );
    return {};
  }

  long  DOCX::Attach()
  {
    return ++refCnt;
  }

  long  DOCX::Detach()
  {
    auto  rCount = --refCnt;

    if ( rCount == 0 )
    {
      if ( !string.empty() )
        output->AddBlock( string );

      delete this;
    }
    return rCount;
  }

  // Para implementation

  Para::~Para()
  {
    if ( !string.empty() )
      output->AddMarkupTag( tagStr )->AddBlock( string );
  }

  auto  Para::AddMarkupTag( const std::string_view& tag, const markup_attribute& att ) -> mtc::api<IText>
  {
    static const std::initializer_list<const char*> ignoreTags = {
      "w:pPr",
      "w:r",
      "w:t" };

  // check ignore tags
    for ( auto ignore : ignoreTags )
      if ( tag == ignore )  return this;

    if ( tag == "w:p" )
    {
      assert( false );
    }

    if ( tag == "w:pStyle" )
    {
      auto  pstyle = att.find( "w:val" );

      if ( pstyle != att.end() && !pstyle->second.empty() )
      {
        if ( mtc::w_strcasecmp( pstyle->second.c_str(), "title" ) == 0 )
        {
          tagStr = "title";
        }
          else
        if ( mtc::w_strncasecmp( pstyle->second.c_str(), "heading", 7 ) == 0 )
        {
          tagStr = "h" + pstyle->second.substr( 7 );
        }
      }
      return this;
    }
    return nullptr;
  }

  auto  Para::AddParagraph( const Paragraph& para ) -> Paragraph
  {
    uint32_t  coding = para.GetEncoding();

    if ( coding == uint32_t(-1) ) string += para.GetWideStr();
      else  string += codepages::mbcstowide( coding, para.GetCharStr() );
    return {};
  }

  // public call method

  int   ParseDOCX( IText* text, const mtc::api<const mtc::IByteBuffer>& buff )
  {
    if ( text != nullptr )
    {
      auto  zarc = OpenZip( buff );
      auto  zsrc = zarc->GetFile( "word/document.xml" );

      if ( zsrc != nullptr )
      {
        auto  xt = DOCX( text, 1 );

        ParseXML( &xt, zsrc.ptr() );

        return 0;
      }
      throw std::invalid_argument( "archive does not contain 'contents.xml'" );
    }
    throw std::invalid_argument( "undefined output" );
  }

}
