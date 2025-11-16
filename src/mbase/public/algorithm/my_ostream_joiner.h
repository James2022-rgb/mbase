#pragma once

#include <iterator>
#include <ostream>
#include <string>
#include <sstream>
#include <algorithm>

// TODO: stop deriving from std::iterator; the C++17 standard deprecates it.
#if defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace mbase {

// An implementation of std::experimental::ostream_joiner. VS2017 doesn't provide one.
template <class T, class charT = char, class traits = std::char_traits<charT> >
class my_ostream_joiner : public std::iterator<std::output_iterator_tag, void, void, void, void>
{
  std::basic_ostream<charT, traits>* out_stream;
  std::string delim;
  bool first_elem;
public:
  using char_type = charT;
  using traits_type = traits;
  using ostream_type = std::basic_ostream<charT, traits>;
  using _Unchecked_type = iterator;
  explicit my_ostream_joiner(ostream_type& s) : out_stream(&s), delim(", "), first_elem(true) {}
  my_ostream_joiner(ostream_type& s, const std::string& delimiter)
    : out_stream(&s), delim(delimiter), first_elem(true) { }
  my_ostream_joiner(const my_ostream_joiner<T, charT, traits>& x)
    : out_stream(x.out_stream), delim(x.delim), first_elem(x.first_elem) {}
  ~my_ostream_joiner() {}

  my_ostream_joiner<T, charT, traits>& operator= (const T& value) {
    if (!first_elem && !delim.empty())
      *out_stream << delim;
    *out_stream << value;
    first_elem = false;
    return *this;
  }

  my_ostream_joiner<T, charT, traits>& operator*() { return *this; }
  my_ostream_joiner<T, charT, traits>& operator++() { return *this; }
  my_ostream_joiner<T, charT, traits>& operator++(int) { return *this; }
};

#if _MSC_VER >= 1400
/*template<class T, class charT, class traits>
struct std::_Is_checked_helper<my_ostream_joiner<T, charT, traits>> : public std::true_type {
  // mark my_ostream_joiner as checked
};*/
#endif

template<class Container>
std::string JoinElements(Container const& elements, std::string const& delim = ", ") {
  std::ostringstream oss;
  my_ostream_joiner<typename Container::value_type> joiner(oss, delim);
  std::copy(std::begin(elements), std::end(elements), joiner);
  return oss.str();
}

template<class Container>
std::string JoinElementsInReverse(Container const& elements, std::string const& delim = ", ") {
  std::ostringstream oss;
  my_ostream_joiner<typename Container::value_type> joiner(oss, delim);
  std::copy(std::rbegin(elements), std::rend(elements), joiner);
  return oss.str();
}

template<class Container, class Predicate>
std::string JoinElementsWithPredicate(Container const& elements, Predicate const& predicate, std::string const& delim = ", ") {
  std::ostringstream oss;
  my_ostream_joiner<std::remove_reference_t<decltype(predicate(typename Container::value_type()))>> joiner(oss, delim);
  std::transform(std::begin(elements), std::end(elements), joiner, predicate);
  return oss.str();
}

}

#if defined(__clang__)
# pragma clang diagnostic pop
#endif
