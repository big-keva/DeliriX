# include "../formats.hpp"
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

    auto  AddMarkupTag( const std::string_view&, const markup_attribute& ) -> mtc::api<IText>  override;
    auto  AddParagraph( const Paragraph& ) -> Paragraph override;

    long  Attach() override;
    long  Detach() override;

  };

  // FB2 implementation

  auto  FB2::AddMarkupTag( const std::string_view& tag, const markup_attribute& att ) -> mtc::api<IText>
  {
    if ( tag == "binary" )
      return nullptr;
    if ( tag == "FictionBook" )
      return this;

    if ( tag == "p" )
    {
      if ( !string.empty() )
      {
        output->AddBlock( string );
        string.clear();
      }
      return this;
    }
    return new FB2( output->AddMarkupTag( tag, att ) );
  }

  auto  FB2::AddParagraph( const Paragraph& para ) -> Paragraph
  {
    auto  coding = para.GetEncoding();

    if ( coding == uint32_t(-1) ) string += para.GetWideStr();
      else  string += codepages::mbcstowide( coding, para.GetCharStr() );
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
        output->AddBlock( string );

      delete this;
    }
    return rCount;
  }

  int   ParseFB2( IText* text, const mtc::api<const mtc::IByteBuffer>& buff )
  {
    if ( text != nullptr && buff != nullptr )
    {
      auto  xt = FB2( text, 1 );

      ParseXML( &xt, buff.ptr() );

      return 0;
    }
    throw std::invalid_argument( "undefined output" );
  }

}
