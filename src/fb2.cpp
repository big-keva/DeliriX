
# include "../archive.hpp"
# include "../formats.hpp"
# include "xml.hpp"
# include <moonycode/codes.h>
# include <mtc/wcsstr.h>

namespace DeliriX
{

  class FB2 final: public IText
  {
    mtc::api<IText>   output;

    std::atomic_long  refCnt;

    mtc::widestr      string;

  public:
    FB2( IText* tx ): output( tx ), refCnt( 0 ) {}
    FB2( IText* tx, int inplace ): output( tx ), refCnt( inplace ) {}

    auto  AddMarkupTag( const char_string_view&, const markup_attribute& ) -> mtc::api<IText>  override;
    auto  AddParagraph( const char_string_view&, uint32_t = default_codepage ) -> Paragraph override;
    auto  AddParagraph( const wide_string_view& ) -> Paragraph override;
    auto  AddParagraph( const Paragraph& ) -> Paragraph override;

    long  Attach() override;
    long  Detach() override;

  };

  // FB2 implementation

  auto  FB2::AddMarkupTag( const char_string_view& tag, const markup_attribute& att ) -> mtc::api<IText>
  {
    if ( tag == "binary" )
      return nullptr;
    if ( tag == "FictionBook" )
      return this;

    if ( tag == "p" )
    {
      if ( !string.empty() )
      {
        output->AddParagraph( string );
        string.clear();
      }
      return this;
    }
    return new FB2( output->AddMarkupTag( tag, att ) );
  }

  auto  FB2::AddParagraph( const char_string_view& str, uint32_t enc ) -> Paragraph
  {
    string += codepages::mbcstowide( enc, str );
    return {};
  }

  auto  FB2::AddParagraph( const wide_string_view& str ) -> Paragraph
  {
    string += str;
    return {};
  }

  auto  FB2::AddParagraph( const Paragraph& para ) -> Paragraph
  {
    uint32_t  encode;

    if ( (encode = para.GetEncoding()) == uint32_t(-1) ) string += para.GetWideStr();
      else  string += codepages::mbcstowide( encode, para.GetCharStr() );
    return {};
  }

  long  FB2::Attach()
  {
    return ++refCnt;
  }

  long  FB2::Detach()
  {
    auto  rCount = --refCnt;

    if ( rCount == 0 )
    {
      if ( !string.empty() )
        output->AddParagraph( string );

      delete this;
    }
    return rCount;
  }

  int   ParseFB2( IText* text, const mtc::api<const mtc::IByteBuffer>& buff )
  {
    if ( text != nullptr && buff != nullptr )
    {
      auto  xt = FB2( text, 1 );

      tinyxml::ParseXML( &xt, buff.ptr() );

      return 0;
    }
    throw std::invalid_argument( "undefined output" );
  }

}
