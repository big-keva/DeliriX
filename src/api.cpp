# include "../DOM-text.hpp"
# include <moonycode/codes.h>
# include <mtc/wcsstr.h>
# include <functional>

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

  Paragraph::Paragraph( const char* str, uint32_t len, uint32_t enc )
  {
    if ( len == (uint32_t)-1 )
      for ( len = 0; str[len] != 0; ++len ) (void)NULL;
    charstr = (char*)(1 + ParagraphCtl::Create( str, len, enc ));
  }

  Paragraph::Paragraph( const widechar* str, uint32_t len )
  {
    if ( len == (uint32_t)-1 )
      for ( len = 0; str[len] != 0; ++len ) (void)NULL;
    widestr = (widechar*)(1 + ParagraphCtl::Create( str, len ));
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

  bool  Paragraph::Serialize( std::function<bool( const void*, size_t )> fns ) const
  {
    auto  enc = GetEncoding();
    auto  len = GetTextSize();
    auto  out = ::Serialize( ::Serialize( &fns, enc + 1 ), len );

    out = enc == (uint32_t)-1 ? ::Serialize( out, charstr, len * sizeof(widechar) )
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
    auto  AddParagraph( const char_string_view& str, unsigned enc ) -> Paragraph override
    {
      widechar  wcsstr[0x400];

      if ( enc != codepages::codepage_utf8 && str.length() <= std::size( wcsstr ) )
        return output->AddParagraph( wide_string_view{ wcsstr, codepages::mbcstowide( enc, wcsstr, str.data(), str.length() ) } );
      return output->AddParagraph( codepages::mbcstowide( enc != 0 ? enc : encode, str ) );
    }
    auto  AddParagraph( const wide_string_view& str ) -> Paragraph override
    {
      return output->AddParagraph( str );
    }
    auto  AddParagraph( const Paragraph& ) -> Paragraph override
    {
      throw std::logic_error( "not implemented" );
    }

  protected:
    mtc::api<IText> output;
    unsigned        encode;

  };

  bool  IsEncoded( const ITextView& textview, unsigned codepage )
  {
    for ( auto& next: textview.GetBlocks() )
      if ( next.GetEncoding() != codepage )
        return false;
    return true;
  }

  auto  CopyUtf16( IText* output, const ITextView& source, unsigned encode ) -> IText*
  {
    UtfTxt  utfOut( output, encode );
      utfOut.Attach();
    return Serialize( &utfOut, source );
  }

  auto  Serialize( IText* output, const ITextView& source ) -> IText*
  {
    auto  lineIt = source.GetBlocks().begin();
    auto  spanIt = source.GetMarkup().begin();
    auto  offset = uint32_t(0);
    auto  fPrint = std::function<void( IText*, uint32_t )>();

    fPrint = [&]( IText* to, uint32_t up )
    {
      while ( lineIt != source.GetBlocks().end() && offset < up )
      {
        // check if print next line to current IText*
        if ( spanIt == source.GetMarkup().end() || offset < spanIt->uLower )
        {
          auto  enc = lineIt->GetEncoding();

          if ( enc == unsigned(-1) )
            to->AddParagraph( lineIt->GetWideStr() );
          else
            to->AddParagraph( lineIt->GetCharStr(), enc );

          offset += lineIt->GetTextSize();

          if ( ++lineIt == source.GetBlocks().end() )  return;
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

}
