/* Do not remove the headers from this file! see /USAGE for more info. */

string chr( int i );

nosave private string ascii = "";
nosave private string whitespace = " \t\n\r";
nosave private string lowercase = "abcdefghijklmnopqrstuvwxyz";
nosave private string uppercase = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";


nosave private string _lower, _upper, _swapcase;

protected void create()
{
  int i;

  for(i=0; i < 256; i++){
     if(i==0)ascii+=" ";
	ascii += chr(i);
  }

  _lower = ascii[0..('A'-1)] + lowercase + ascii[('Z'+1)..]; 
  _upper = ascii[0..('a'-1)] + uppercase + ascii[('z'+1)..];
  _swapcase = _upper[0..'A'-1] + lowercase + _upper['Z'+1..];
}

// TODO Check for proper names ?
string add_article(string str)
{
  switch (str[0])
  {
    case 'a':
    case 'e':
    case 'i':
    case 'o':
    case 'u':
      return "an "+str;
  }
  return "a "+str;
}

private int strstr(string s, string lookfor, int start)
{
  int x;

  x = strsrch(s[start..], lookfor);
  if ( x == -1 )
    return -1;
  return start + x;
}


#define DEFAULT_WIDTH	75
#define DEFAULT_INDENT	4

//:FUNCTION wrap
// Wrap function from Jihad @ Lima Bean.  Takes the string to wrap,
// and optionally a width to wrap it to and optionally an indent for
// any line after the first.
varargs string wrap(string str, int width, int indent)
{
  if ( !indent )
    return sprintf("%-=*s", width || DEFAULT_WIDTH, str);

  return sprintf(
      "%s%-=*s",
      str[0..indent - 1],
      (width || DEFAULT_WIDTH) - indent,
      str[indent..]
      );
}

string iwrap(string str)
{
  return wrap(str, 0, DEFAULT_INDENT);
}

string trim_spaces( string s ){
  int i, j;

  j = strlen(s) - 1;
  if ( j < 0 )
    return "";

   while( i <= j && member_array(s[i], whitespace) != -1 )
     i++;
  while( i <= j && member_array(s[j], whitespace) != -1 )
    j--;
  return s[i..j];
}

string expand_range(string str)
{
  string ret;
  mixed  tmp;
  int	 i,x,y;

  if(!stringp(str))
    return 0;
  ret = "";
  str=implode(explode(str," "),"");
  tmp = explode( str, "," );
  i = sizeof( tmp );

  while( i-- )
    if( sscanf( tmp[i], "%d-%d", x, y ) == 2 )
      while( x <= y )
        ret = set_bit( ret, x ), x++;
     else
       ret = set_bit( ret, to_int(tmp[i]) );
  return ret;
}

// Rust
string chr( int i )
{
  return sprintf("%c",i);
}

int ord( string x )
{
  if( strlen( x ) != 1 )
    error("bad type arg to simul_efun ord()");
  return x[0];
}

string swapcase( string s )
{
  return map(s, (: _swapcase[$1] :) );
}

varargs int find( string s, string sub, int i )
{
	if( i < 0 )
	  i += strlen(s);
	return strstr( s, sub, i );
}

varargs int rfind( string s, string sub, int i )
{
  int j, k;
  
  j = -1;
  k = strlen(s);

  if(i<0)
    i+=k;
  while((i<k)&&(i=strstr(s,sub,i))!=-1)
  {
    j=i;
    i++;
  }

  return j;
}

varargs int count_substrings( string s, string sub, int i )
{
  int n, m, r;

  if(i<0)
    i+= strlen(s);
  n = strlen(sub);
  m = strlen(s) + 1 - n;
  if( !n )
    return m-i;
  n--;

  r = 0;
  while( i<m )
  {
    if( sub == s[i..(i+n)])
    {
      r++;
      i+=n;
    } else {
      i++;
    }
  }
  return r;
}

string power_str( string s, int t )
{
	return repeat_string(s,t);
}

string ljust( string s, int width )
{
  int n;

  n = width - strlen(s);
  if( n <= 0 )
    return s;

  return s + power_str(" ",n);
}

string rjust( string s, int width )
{
  int n;

   n = width - strlen(s);
  if(n<=0)
    return s;
  return power_str(" ",n) + s;
}

string center( string s, int width )
{
  int n, half;

  n = width - strlen(s);
  if( n <= 0 ) return s;
  half = n>>1;
  if( n%2&&width%2)
    half++;
  return power_str(" ",half) + s + power_str(" ",n-half);
}

// Note that truncate adds an elipsis ("...") to the end if it 
// truncates.
string truncate( string s, int length )
{
  length -= 3;
  if(length < 1)
		error("truncate:  value too low");

  if(strlen(s) <= length)
    return s;
  return s[0..length-1] + "...";
}

int is_letter(int c)
{
  return strsrch(lowercase+uppercase,c) != -1;
}

string format_list(string *list, string separator)
{
  if (!separator)
    separator = "and";
  if (sizeof(list)==0)
    return "";
  if (sizeof(list)==1)
    return list[0];
  if (sizeof(list)==2)
    return list[0] + " " + separator + " " + list[1];
  return implode(list[0..<2], ", ") + ", " + separator + " " + list[<1];
}

string tidy_list(mixed items, string separator)
{
  mapping tmp = ([]);
  string *list = ({});

  foreach(mixed item in items)
  {
    if(objectp(item))
      item=item->short();
    if(!stringp(item))
      continue;
    if(member_array(item, keys(tmp))<0)
      tmp += ([item:1]);
    else
      tmp[item]++;
  }
  foreach(string name, int num in tmp)
  {
    if(num == 1)
      list += ({ add_article(name) });
    else
      list += ({ sprintf("%d %s", num, name) });
  }
  return format_list(list, separator);
}

//:FUNCTION title_capitalize
//Return the incoming string with the appropriate words capitalized
//for a title.
string title_capitalize(string instring)
{
/* Always capitalize the first word. The normal capitalize() handles this */
  string *inarr=explode(capitalize(instring)," ");
  string *out=({});
/* There are more in parse_command_prepos_list() than is necessary, but 
 * they are all two word preps, so they will never be matched, thus this 
 * works */
#define UNCAPPED ({ "a","an","the","and","but","or",}) + master()->parse_command_prepos_list()

  foreach(string word in inarr)
  {
    if(member_array(word,UNCAPPED)>-1)
      out+=({word});
    else
      out+=({capitalize(word)});
  }

  return implode(out," ");
}
