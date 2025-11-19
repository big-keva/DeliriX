
# include "../archive.hpp"
# include "../formats.hpp"
# include <moonycode/codes.h>
# include <mtc/wcsstr.h>

namespace DeliriX
{

  class ODT final: public IText
  {
    mtc::api<IText>   output;

    std::atomic_long  refCnt;

    mtc::widestr      string;

  public:
    ODT( IText* tx ): output( tx ), refCnt( 0 ) {}
    ODT( IText* tx, int inplace ): output( tx ), refCnt( inplace ) {}

    auto  AddMarkupTag( const std::string_view&, const markup_attribute& ) -> mtc::api<IText>  override;
    auto  AddParagraph( const Paragraph& ) -> Paragraph override;

    long  Attach() override;
    long  Detach() override;

  };

  // ODT implementation

  auto  ODT::AddMarkupTag( const std::string_view& tag, const markup_attribute& att ) -> mtc::api<IText>
  {
    static const std::initializer_list<const char*> ignoreTags = {
      "office:document-content",
      "office:body",
      "office:text",
      "text:span",
      "text:a" };
    static const std::initializer_list<std::pair<const char*, const char*>> renameTags = {
      { "table:table-cell", "td" },
      { "table:table-row", "tr" },
      { "table:table", "table" },
      { "text:list-item", "li" },
      { "text:list", "ul" },
      { "text:p", "p" } };

  // check ignored tags
    for ( auto& ignore : ignoreTags )
      if ( tag == ignore )  return this;

  // check renamed tags
    for ( auto& rename : renameTags )
      if ( tag == rename.first )  return new ODT( output->AddMarkupTag( rename.second, att ) );

    // headings
    if ( tag == "text:h" )
    {
      auto  outLevel = att.find( "text:outline-level" );
      auto  tagValue = std::string( "h" );

      if ( outLevel != att.end() )
        tagValue += outLevel->second;

      return new ODT( output->AddMarkupTag( tagValue, {} ) );
    }

  // spaces
    if ( tag == "text:s" )
    {
      auto  getCount = att.find( "text:c" );
      auto  strCount = getCount != att.end() ? getCount->second : "1";
      auto  intCount = strtol( strCount.c_str(), nullptr, 10 );
      auto  strPaste = mtc::strprintf( mtc::strprintf( "%%%dc", std::max( intCount, 1L ) ).c_str(), ' ' );

      return AddBlock( strPaste ), this;
    }

    return new ODT( output->AddMarkupTag( tag, att ) );
  }

  auto  ODT::AddParagraph( const Paragraph& para ) -> Paragraph
  {
    auto  coding = para.GetEncoding();

    if ( coding == uint32_t(-1) ) string += para.GetWideStr();
      else string += codepages::mbcstowide( coding, para.GetCharStr() );
    return {};
  }

  long  ODT::Attach()
  {
    return ++refCnt;
  }

  long  ODT::Detach()
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

  int   ParseODT( IText* text, const mtc::api<const mtc::IByteBuffer>& buff )
  {
    if ( text != nullptr )
    {
      auto  zarc = OpenZip( buff );
      auto  zsrc = zarc->GetFile( "content.xml" );

      if ( zsrc != nullptr )
      {
        auto  xt = ODT( text, 1 );

        ParseXML( &xt, zsrc.ptr() );

        return 0;
      }
      throw std::invalid_argument( "archive does not contain 'contents.xml'" );
    }
    throw std::invalid_argument( "undefined output" );
  }

}
