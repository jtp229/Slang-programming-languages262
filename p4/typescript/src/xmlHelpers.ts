/**
 * Remove escape characters from a string when reading from XML
 *
 * @param s The string to unescape
 *
 * @return The unescaped string
 */
export function xmlUnescape(s: string) {
  // We need to go char by char to get it right
  let ans = '';
  let inEscape = false;
  for (let i of s) {
    if (!inEscape) {
      if (i !== '\\') ans += i;
      else inEscape = true;
    } else {
      if (i === '\\') {
        ans += '\\';
        inEscape = false;
      } else if (i === 't') {
        ans += '\t';
        inEscape = false;
      } else if (i === 'n') {
        ans += '\n';
        inEscape = false;
      } else if (i === "'") {
        ans += "'";
        inEscape = false;
      }
      if (inEscape) throw 'Invalid string?!?';
    }
  }
  return ans;
}
