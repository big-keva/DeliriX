# include "../formats.hpp"
# include "../archive.hpp"
# include "../DOM-text.hpp"
# include "../DOM-dump.hpp"
# include "mock-buff.hpp"
# include <mtc/test-it-easy.hpp>
# include <mtc/wcsstr.h>
# include <mtc/zmap.h>

using namespace DeliriX;

extern unsigned char  sample_odtzip_buf[];
extern unsigned       sample_odtzip_len;
extern unsigned char  sample_odtDeliriX_buf[];
extern unsigned       sample_odtDeliriX_len;

TestItEasy::RegisterFunc  test_odt( []()
{
  TEST_CASE( "DeliriX/odt" )
  {
    SECTION( "ODT parser loads .odt documents" )
    {
      SECTION( "with empty buffer or output, it throws std::invalid_argument" )
      {
        Text  text;

        REQUIRE_EXCEPTION( ParseODT( nullptr, mtc::CreateByteBuffer( "aaa", 3 ).ptr() ),
          std::invalid_argument );
        REQUIRE_EXCEPTION( ParseODT( &text, nullptr ),
          std::invalid_argument );
      }
      SECTION( "with correct data, it parses xml" )
      {
        Text  text;

        if ( REQUIRE_NOTHROW( ParseODT( &text, mtc::CreateByteBuffer( sample_odtzip_buf, sample_odtzip_len ).ptr() )))
        {
          std::string tags;

          if ( REQUIRE_NOTHROW( text.Serialize( dump_as::Tags( dump_as::MakeOutput( &tags ) ) ) ) )
          {
            REQUIRE( tags ==
              "<p>\n"
              "  Я Андрей Коваленко. Или Keva.\n"
              "</p>\n"
              "<p>\n"
              "  Сделал самый быстрый и второй по полноте и качеству после ашмановской ОРФО русский морфологический анализатор libmorph (с 1994 по настоящее время).\n"
              "</p>\n"
              "<p>\n"
              "  В 1996 году придумал и спроектировал, а в 1997 вместе с Женей Киреевым (ныне председатель совета директоров Positive Technologies) и Женей Бондаренко (ныне покойным) запустил первый русский поисковик с поддержкой русской морфологии — Апорт!.\n"
              "</p>\n"
              "<p>\n"
              "  В 1998 в Харькове с партнёрами — Юрой Назаровым, Юрой Логвиновым и Лёшей Чуксиным при финансовой поддержке Укрсиббанка запустил клон системы Апорт! — українську пошукову систему &lt;META&gt; и занялся разработкой нового большого поискового движка. В чуть разных ипостасях он стал и Рамблером от 2000 года и до его конца, и развитием &lt;МЕТЫ&gt;.\n"
              "</p>\n"
              "<p>\n"
              "  Сделал в 2005-2006 году для первого новостного кластеризатора Новотека подсистему построения дайджеста новостного кластера в развитии события.\n"
              "</p>\n"
              "<p>\n"
              "  С 2016 по конец 2024 года работал с компанией «Новые Облачные Технологии», ныне «МойОфис». Сделал корпоративную поисковую систему, интегрированную с почтой Mailion и корпоративным мессенджером Squadus. Система произвольно масштабируется, поддерживает репликацию, в том числе и на несколько data-центров.\n"
              "</p>\n"
              "<p>\n"
              "  Излюбленные способы выражать свои мысли — C++ и русский язык. Хотя бегло говорю и внятно пишу по-английски, но это от частых путешествий.\n"
              "</p>\n"
              "<p>\n"
              "  Есть красотка-жена, пятеро детей и собака.\n"
              "</p>\n"
              "<p>\n"
              "  Живу в ближнем пригороде. Люблю вина происхождением из ЮАР. Гоняю самогон, когда слива поспевает.\n"
              "</p>\n"
              "<p>\n"
              "  Качок, хотя уже не в обойме.\n"
              "</p>\n"
              "<p>\n"
              "  Сейчас свободен, хочу пообщаться и, возможно,  вместе с вами что-то ещё сделать.\n"
              "</p>\n"
              "<p>\n"
              "  keva@rambler.ru\n"
              "</p>\n"
              "<p>\n"
              "  +7(926)513-2991\n"
              "</p>\n"
              "<p>\n"
              "  @Big_keva\n"
              "</p>\n"
              "<p>\n"
              "  https://github.com/big-keva/\n"
              "</p>\n"
              "<p>\n"
              "  https://vk.com/bigkeva\n"
              "</p>\n" );
          }
        }
        text.clear();
        if ( REQUIRE_NOTHROW( ParseODT( &text, mtc::CreateByteBuffer( sample_odtDeliriX_buf, sample_odtDeliriX_len ).ptr() ) ) )
        {
          std::string tags;

          if ( REQUIRE_NOTHROW( text.Serialize( dump_as::Tags( dump_as::MakeOutput( &tags ) ) ) ) )
            REQUIRE( tags ==
              "<p>\n"
              "  DeliriX\n"
              "</p>\n"
              "<h1>\n"
              "  Что это такое?\n"
              "</h1>\n"
              "<p>\n"
              "  DeliriX - библиотека поддержки популярных форматов офисных файлов. Обеспечивает чтение видимого пользователю содержимого, передачу в декларированный интерфейс IText и, при желании, выделение некоторых метаданных в виде ассоциативного массива key-value (mtc::zmap).\n"
              "</p>\n"
              "<h2>\n"
              "  Интерфейс документа\n"
              "</h2>\n"
              "<p>\n"
              "  Клиенское приложение предоставляет функциям разбора форматов интерфейс текстового документа, позволяющий добавлять текстовые фрагменты (строки) или блочную разметку. В последнем случае результатом должен быть такой же интерфейс IText, обеспечивающий добавление текстовых фрагментов в этот блок.\n"
              "</p>\n"
              "<table>\n"
              "  <tr>\n"
              "    <td>\n"
              "      <p>\n"
              "        struct IText: mtc::Iface\n"
              "      </p>\n"
              "      <p>\n"
              "      </p>\n"
              "      {\n"
              "      <p>\n"
              "          auto  AddMarkupTag( const char*, size_t = -1 ) → mtc::api&lt;IText&gt;;\n"
              "      </p>\n"
              "      <p>\n"
              "          auto AddParagraph( const char*, uint32_t, uint32_t ) -&gt; Paragraph;\n"
              "      </p>\n"
              "      <p>\n"
              "          auto  AddParagraph( const widechar*, uint32_t ) -&gt; Paragraph;\n"
              "      </p>\n"
              "      <p>\n"
              "          auto  AddParagraph( const Paragraph&amp; )          -&gt; Paragraph;\n"
              "      </p>\n"
              "      <p>\n"
              "        };\n"
              "      </p>\n"
              "    </td>\n"
              "  </tr>\n"
              "</table>\n"
              "<p>\n"
              "  Методы:\n"
              "</p>\n"
              "<ul>\n"
              "  <li>\n"
              "    <p>\n"
              "      AddMarkupTag\n"
              "    </p>\n"
              "  </li>\n"
              "  <li>\n"
              "    <p>\n"
              "      AddParagraph\n"
              "    </p>\n"
              "  </li>\n"
              "</ul>\n"
              "<h3>\n"
              "  Функции разбора форматов\n"
              "</h3>\n"
              "<p>\n"
              "  Для каждого из поддерживаемых форматов предоставляется функция разбора, например, ParseODT( IText*, … ). В случае, если формат заранее не известен, можно положиться на автоматическое определение работающее с некоторой точностью.\n"
              "</p>\n"
              "<table>\n"
              "  <tr>\n"
              "    <td>\n"
              "      <p>\n"
              "        Функция разбора\n"
              "      </p>\n"
              "    </td>\n"
              "    <td>\n"
              "      <p>\n"
              "        Форматы документов\n"
              "      </p>\n"
              "    </td>\n"
              "  </tr>\n"
              "  <tr>\n"
              "    <td>\n"
              "      <p>\n"
              "        ParseODT\n"
              "      </p>\n"
              "    </td>\n"
              "    <td>\n"
              "      <p>\n"
              "        OpenOffice Text\n"
              "      </p>\n"
              "    </td>\n"
              "  </tr>\n"
              "  <tr>\n"
              "    <td>\n"
              "      <p>\n"
              "        ParseDOC\n"
              "      </p>\n"
              "    </td>\n"
              "  </tr>\n"
              "  <tr>\n"
              "    <td>\n"
              "      <p>\n"
              "        ParseDOCX\n"
              "      </p>\n"
              "    </td>\n"
              "  </tr>\n"
              "  <tr>\n"
              "    <td>\n"
              "      <p>\n"
              "        ParseRTF\n"
              "      </p>\n"
              "    </td>\n"
              "  </tr>\n"
              "</table>\n" );
        }
      }
    }
  }
} );
