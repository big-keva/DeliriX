# include "../DOM-text.hpp"
# include <moonycode/codes.h>
# include <mtc/wcsstr.h>
# include <functional>
#include <bits/ios_base.h>

using SerializeFn = std::function<bool( const void*, size_t )>;
using FetchFromFn = std::function<bool( void*, size_t )>;

template <>
SerializeFn*  Serialize( SerializeFn* o, const void* p, size_t l )
  {  return o != nullptr && (*o)( p, l ) ? o : nullptr;  }

template <>
FetchFromFn*  FetchFrom( FetchFromFn* s, void* p, size_t l )
  {  return s != nullptr && (*s)( p, l ) ? s : nullptr;  }

namespace DeliriX
{
  struct ParagraphCtl
  {
    uint32_t  encode;
    uint32_t  length;
    int       rcount;

    static  ParagraphCtl* Create( const char* str, uint32_t len, uint32_t enc )
    {
      auto  nalloc = (sizeof(ParagraphCtl) * 2 + len) / sizeof(ParagraphCtl);
      auto  palloc = new ParagraphCtl[nalloc];

      ((char*)(1 + new ( palloc ) ParagraphCtl{ enc, len, 1 }))[len] = 0;

      if ( str != nullptr )
        mtc::w_strncpy( (char*)(1 + palloc), str, len );

      return palloc;
    }
    static  ParagraphCtl* Create( const widechar* str, uint32_t len )
    {
      auto  nalloc = (sizeof(ParagraphCtl) * 2 + (len + 1) * sizeof(widechar) - 1) / sizeof(ParagraphCtl);
      auto  palloc = new ParagraphCtl[nalloc];

      ((widechar*)(1 + new ( palloc ) ParagraphCtl{ uint32_t(-1), len, 1 }))[len] = 0;

      if ( str != nullptr )
        mtc::w_strncpy( (widechar*)(1 + palloc), str, len )[len] = 0;

      return palloc;
    }
  };

  // Paragraph implementation

  Paragraph::Paragraph(): charstr( nullptr )
  {
  }

  Paragraph::~Paragraph()
  {
    if ( charstr != nullptr && --((ParagraphCtl*)charstr)[-1].rcount == 0 )
      delete[] (-1 + (ParagraphCtl*)charstr);
  }

  Paragraph::Paragraph( const Paragraph& p ): charstr( p.charstr )
  {
    if ( charstr != nullptr )
      ++((ParagraphCtl*)charstr)[-1].rcount;
  }

  Paragraph::Paragraph( Paragraph&& p ): charstr( p.charstr )
  {
    p.charstr = nullptr;
  }

  Paragraph& Paragraph::operator = ( const Paragraph& p )
  {
    if ( charstr != nullptr && --((ParagraphCtl*)charstr)[-1].rcount == 0 )
      delete[] ((ParagraphCtl*)charstr - 1);
    if ( (charstr = p.charstr) != nullptr )
      ++((ParagraphCtl*)charstr)[-1].rcount;
    return *this;
  }

  Paragraph& Paragraph::operator = ( Paragraph&& p )
  {
    if ( charstr != nullptr && --((ParagraphCtl*)charstr)[-1].rcount == 0 )
      delete[] ((ParagraphCtl*)charstr - 1);
    charstr = p.charstr;
      p.charstr = nullptr;
    return *this;
  }

  uint32_t  Paragraph::GetEncoding() const
  {
    return charstr != nullptr ? ((ParagraphCtl*)charstr)[-1].encode : 0;
  }

  uint32_t  Paragraph::GetTextSize() const
  {
    return charstr != nullptr ? ((ParagraphCtl*)charstr)[-1].length : 0;
  }

  auto  Paragraph::GetCharStr() const -> std::string_view
  {
    if ( charstr != nullptr && GetEncoding() != uint32_t(-1) )
      return { charstr, GetTextSize() };
    return { nullptr, 0 };
  }

  auto  Paragraph::GetWideStr() const -> std::basic_string_view<widechar>
  {
    if ( widestr != nullptr && GetEncoding() == uint32_t(-1) )
      return { widestr, GetTextSize() };
    return { nullptr, 0 };
  }

  auto  Paragraph::GetBufLen() const -> size_t
  {
    auto  enc = GetEncoding();
    auto  len = GetTextSize();
    auto  cch = ::GetBufLen( enc + 1 )
              + ::GetBufLen( len );

    return enc == uint32_t(-1) ? cch + len * sizeof(widechar) : cch + len;
  }

  bool  Paragraph::Serialize( std::function<bool( const void*, size_t )> fns ) const
  {
    auto  enc = GetEncoding();
    auto  len = GetTextSize();
    auto  out = ::Serialize( ::Serialize( &fns, enc + 1 ), len );

    out = enc == (uint32_t)-1 ? ::Serialize( out, widestr, len * sizeof(widechar) )
      : ::Serialize( out, charstr, len );

    return out != nullptr;
  }

  bool  Paragraph::FetchFrom( std::function<bool( void*, size_t )> fns )
  {
    uint32_t  enc;
    uint32_t  len;

    auto  src = ::FetchFrom( ::FetchFrom( &fns, enc ), len );
      --enc;

    if ( enc == (uint32_t)-1 )
      src = ::FetchFrom( src, (void*)(widestr = (const widechar*)(1 + ParagraphCtl::Create( nullptr, len ))), len * sizeof(widechar) );
    else
      src = ::FetchFrom( src, (void*)(widestr = (const widechar*)(1 + ParagraphCtl::Create( nullptr, len, enc ))), len );

    return src != nullptr;
  }

  // IText

  auto  IText::AddBlock( const widechar* str, uint32_t len ) -> Paragraph
  {
    Paragraph para;

    if ( len == uint32_t(-1) )
      for ( len = 0; str[len] != 0; ++len )
        (void)NULL;

    para.widestr = (widechar*)(1 + ParagraphCtl::Create( str, len ));

    return AddParagraph( para );
  }

  auto  IText::AddBlock( uint32_t cp, const char* str, uint32_t len ) -> Paragraph
  {
    Paragraph para;

    if ( len == uint32_t(-1) )
      for ( len = 0; str[len] != 0; ++len )
        (void)NULL;

    para.charstr = (char*)(1 + ParagraphCtl::Create( str, len, cp ));

    return AddParagraph( para );
  }

  // ITextView

  auto  ITextView::GetBufLen() const -> size_t
  {
    auto  blocks = GetBlocks();
    auto  markup = GetMarkup();
    auto  length = ::GetBufLen( blocks.size() )
                 + ::GetBufLen( markup.size() ) + ::GetBufLen( GetLength() );

    for ( auto& str: blocks )
      length += str.GetBufLen();

    for ( auto& tag: markup )
      length += ::GetBufLen( tag.tagKey ) + ::GetBufLen( tag.uLower ) + ::GetBufLen( tag.uUpper );

    return length;
  }

  auto  ITextView::Serialize( IText* output ) const -> IText*
  {
    auto  blocks = GetBlocks();
    auto  markup = GetMarkup();
    auto  lineIt = blocks.begin();
    auto  spanIt = markup.begin();
    auto  offset = uint32_t(0);
    auto  fPrint = std::function<void( IText*, uint32_t )>();

    fPrint = [&]( IText* to, uint32_t up )
    {
      while ( lineIt != blocks.end() && offset < up )
      {
        // check if print next line to current IText*
        if ( spanIt == markup.end() || offset < spanIt->uLower )
        {
          auto  enc = lineIt->GetEncoding();

          if ( enc == unsigned(-1) )
            to->AddBlock( lineIt->GetWideStr() );
          else
            to->AddBlock( enc, lineIt->GetCharStr() );

          offset += lineIt->GetTextSize();

          if ( ++lineIt == blocks.end() )
            return;
          continue;
        }

        // check if open new span
        if ( offset >= spanIt->uLower )
        {
          auto  new_to = to->AddMarkupTag( spanIt->tagKey );
          auto  uUpper = spanIt->uUpper;
            ++spanIt;
          fPrint( new_to.ptr(), uUpper );
        }
      }
    };

    return fPrint( output, uint32_t(-1) ), output;
  }

  // helpers

  class UtfTxt final: public IText
  {
    implement_lifetime_control

  public:
    UtfTxt( mtc::api<IText> tx, unsigned cp ):
      output( tx ),
      encode( cp )  {}

    auto  AddMarkupTag( const std::string_view& tag, const markup_attribute& ) -> mtc::api<IText> override
    {
      return new UtfTxt( output->AddMarkupTag( tag ), encode );
    }
    auto  AddParagraph( const Paragraph& para ) -> Paragraph override
    {
      auto  coding = para.GetEncoding();

      if ( coding != uint32_t(-1) )
      {
        widechar  wcsstr[0x400];

        if ( coding != codepages::codepage_utf8 && para.GetTextSize() <= std::size( wcsstr ) )
          return output->AddBlock( wcsstr, codepages::mbcstowide( coding, wcsstr, para.GetCharStr() ) );
        return output->AddBlock( codepages::mbcstowide( codepages::codepage_utf8, para.GetCharStr() ) );
      }
      return output->AddParagraph( para );
    }

  protected:
    mtc::api<IText> output;
    unsigned        encode;

  };

  bool  IsEncoded( const ITextView& textview, uint32_t codepage )
  {
    for ( auto& next: textview.GetBlocks() )
      if ( next.GetEncoding() != codepage )
        return false;
    return true;
  }

  auto  CopyUtf16( IText* output, const ITextView& source, uint32_t encode ) -> IText*
  {
    UtfTxt  utfOut( output, encode );
      utfOut.Attach();
    return Serialize( (IText*)&utfOut, source );
  }

}
