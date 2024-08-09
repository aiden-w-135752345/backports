#ifndef STRING_VIEW_HPP
#define STRING_VIEW_HPP 1
#include <string>
#include <ostream>
#include <functional>
#include "type_traits.hpp"
namespace backports{
    template<class T, class Traits = std::char_traits<T>>class basic_string_view{
        static_assert(is_trivial_v<T> && is_standard_layout_v<T> && !is_array_v<T>,"Invalid instantiation of basic_string_view");
        static_assert(is_same_v<T, typename Traits::char_type>,"Invalid instantiation of basic_string_view");
    public:
        using traits_type            = Traits;
        using value_type             = T;
        using pointer                = value_type*;
        using const_pointer          = const value_type*;
        using reference              = value_type&;
        using const_reference        = const value_type&;
        using const_iterator         = const value_type*;// implementation defined type
        using iterator               = const_iterator;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        using reverse_iterator       = const_reverse_iterator;
        using size_type              = size_t;
        using difference_type        = ptrdiff_t;
        static constexpr size_type npos = size_type(-1);
        constexpr basic_string_view() noexcept:len{0}, str{nullptr}{}
        constexpr basic_string_view(const basic_string_view&) noexcept = default;
        constexpr basic_string_view(const_pointer s) noexcept :len{traits_type::length(s)},str{s}{}
        constexpr basic_string_view(const_pointer s, size_type l) noexcept:len{l}, str{s}{}
        constexpr basic_string_view&operator=(const basic_string_view&) noexcept = default;
        constexpr const_iterator begin() const noexcept{ return str; }
        constexpr const_iterator end() const noexcept{ return str + len; }
        constexpr const_iterator cbegin() const noexcept{ return begin(); }
        constexpr const_iterator cend() const noexcept{ return end(); }
        constexpr const_reverse_iterator rbegin() const noexcept{ return const_reverse_iterator(end()); }
        constexpr const_reverse_iterator rend() const noexcept{ return const_reverse_iterator(begin()); }
        constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
        constexpr const_reverse_iterator crend() const noexcept{ return rend(); }
        constexpr size_type size() const noexcept{ return len; }
        constexpr size_type length() const noexcept{ return len; }
        constexpr size_type max_size() const noexcept{
            return (npos - sizeof(size_type) - sizeof(void*))/ sizeof(value_type) / 4;
        }
        constexpr bool empty() const noexcept{ return len == 0; }
        constexpr const_reference operator[](size_type i) const{ return str[i]; }
        constexpr const_reference at(size_type i) const{
            if (i >= len)throw_out_of_range("at",i);
            return str[i];
        }
        constexpr const_reference front() const{ return str[0]; }
        constexpr const_reference back() const{ return str[len-1]; }
        constexpr const_pointer data() const noexcept{ return str; }
        constexpr void remove_prefix(size_type n){ str += n; len -= n; }
        constexpr void remove_suffix(size_type n){ len -= n; }
        constexpr void swap(basic_string_view& that) noexcept{ auto tmp = *this;*this = that;that = tmp; }
        constexpr size_type copy(pointer that, size_type n, size_type i = 0) const{
            if (i > len)throw_out_of_range("copy",i);
            n = std::min(n, len - i);
            traits_type::copy(that, data() + i, n);
            return n;
        }
        constexpr basic_string_view substr(size_type i = 0, size_type n = npos) const{
            if (i > len)throw_out_of_range("substr",i);
            n = std::min(n, len - i);
            return basic_string_view(str + i, n);
        }
        
        constexpr int compare(basic_string_view that) const noexcept{
            int r = traits_type::compare(this->str, that.str, std::min(this->len, that.len));
            if (r == 0)r = (this->len>that.len)-(this->len<that.len);
            return r;
        }
        constexpr int compare(size_type i, size_type n, basic_string_view that) const{ return substr(i, n).compare(that);}
        constexpr int compare(size_type i1, size_type n1, basic_string_view that, size_type i2, size_type n2) const{return substr(i1, n1).compare(that.substr(i2, n2));}
        constexpr int compare(const_pointer that) const noexcept{ return compare(basic_string_view(that)); }
        constexpr int compare(size_type i, size_type n, const_pointer that) const{return compare(i, n, basic_string_view(that));}
        constexpr int compare(size_type i1, size_type n1,const_pointer that, size_type n2) const {return compare(i1, n1, basic_string_view(that, n2));}
        
        constexpr bool starts_with(basic_string_view that) const noexcept{ 
            const auto n = that.len;
            return this->len >= n && traits_type::compare(begin(), that.data(), n) == 0;
        }
        constexpr bool starts_with(value_type that) const noexcept{ return !empty() && traits_type::eq(front(), that); }
        constexpr bool starts_with(const_pointer that) const{ return starts_with(basic_string_view(that)); }
        
        constexpr bool ends_with(basic_string_view that) const noexcept{
            const auto n = that.len;
            return this->len >= n && traits_type::compare(end() - n, that.data(), n) == 0;
        }
        constexpr bool ends_with(value_type that) const noexcept{ return !empty() && traits_type::eq(back(), that); }
        constexpr bool ends_with(const_pointer that) const{ return ends_with(basic_string_view(that)); }
        
        constexpr bool contains(basic_string_view that) const noexcept { return find(that) != npos; }
        constexpr bool contains(value_type that) const noexcept{ return find(that) != npos; }
        constexpr bool contains(const_pointer that) const{ return find(that) != npos; }
        
        constexpr size_type find(basic_string_view that, size_type i = 0) const noexcept{
            if (that.empty()) return i <= len ? i : npos;
            const basic_string_view lead_finder=substr(0,len-that.len+1);
            const value_type lead = that.front();
            for (;i<lead_finder.len;++i){
                i=lead_finder.find(lead,i);
                if (i==npos)return npos;
                if (substr(i).starts_with(that)) return i;
            }
            return npos;
        }
        constexpr size_type find(value_type that, size_type i = 0) const noexcept{
            if (i >= len){return npos;}
            const_pointer p = traits_type::find(str + i, len - i, that);
            if (p)return static_cast<size_type>(p - str);
            return npos;
        }
        constexpr size_type find(const_pointer that, size_type i, size_type n) const{ return find(basic_string_view(that,n), i); }
        constexpr size_type find(const_pointer that, size_type i = 0) const { return find(basic_string_view(that), i); }
        
        constexpr size_type rfind(basic_string_view that, size_type i = npos) const noexcept{
            if(this->len>=that.len)for(i=std::min(this->len-that.len, i)+1;i;)if(substr(--i).starts_with(that))return i;
            return npos;
        }
        constexpr size_type rfind(value_type that, size_type i = npos) const noexcept{
            for(i=std::min(i+1,len);i;)if(traits_type::eq(str[--i], that))return i;
            return npos;
        }
        constexpr size_type rfind(const_pointer that, size_type i, size_type n) const{ return find(basic_string_view(that,n), i); }
        constexpr size_type rfind(const_pointer that, size_type i = npos) const { return rfind(basic_string_view(that), i); }
        
        constexpr size_type find_first_of(basic_string_view that, size_type i = 0) const noexcept{
            if(that.len)for(;i<this->len;++i)if(that.contains(this->str[i]))return i;
            return npos;
        }
        constexpr size_type find_first_of(value_type that, size_type i = 0) const noexcept{ return find(that, i); }
        constexpr size_type find_first_of(const_pointer that, size_type i, size_type n) const{ return find(basic_string_view(that,n), i); }
        constexpr size_type find_first_of(const_pointer that, size_type i = 0) const{ return find_first_of(basic_string_view(that), i); }
        
        constexpr size_type find_last_of(basic_string_view that, size_type i = npos) const noexcept{
            if(that.len)for(i=std::min(this->len,i+1);i;)if(that.contains(this->str[--i]))return i;
            return npos;
        }
        constexpr size_type find_last_of(value_type that, size_type i=npos) const noexcept{ return rfind(that, i); }
        constexpr size_type find_last_of(const_pointer that, size_type i, size_type n) const{ return find(basic_string_view(that,n), i); }
        constexpr size_type find_last_of(const_pointer that, size_type i = npos) const{ return find_last_of(basic_string_view(that), i); }
        
        constexpr size_type find_first_not_of(basic_string_view that, size_type i = 0) const noexcept{
            for (; i < this->len; ++i)if (!that.contains(this->str[i]))return i;
            return npos;
        }
        constexpr size_type find_first_not_of(value_type that, size_type i = 0) const noexcept{
            for (; i < len; ++i)if (!traits_type::eq(str[i], that))return i;
            return npos;
        }
        constexpr size_type find_first_not_of(const_pointer that, size_type i, size_type n) const{ return find(basic_string_view(that,n), i); }
        constexpr size_type find_first_not_of(const_pointer that, size_type i = 0) const { return find_first_not_of(basic_string_view(that), i); }
        
        constexpr size_type find_last_not_of(basic_string_view that, size_type i = npos) const noexcept{
            for(i=std::min(this->len,i+1);i;)if(!that.contains(this->str[--i]))return i;
            return npos;
        }
        constexpr size_type find_last_not_of(value_type that, size_type i = npos) const noexcept{
            for(i = std::min(len,i+1);i;)if(!traits_type::eq(str[--i], that))return i;
            return npos;
        }
        constexpr size_type find_last_not_of(const_pointer that, size_type i, size_type n) const{ return find(basic_string_view(that,n), i); }   
        constexpr size_type find_last_not_of(const_pointer that, size_type i = npos) const{ return find_last_not_of(basic_string_view(that), i); }
    private:
        size_t len;
        const T* str;
        [[noreturn]]void throw_out_of_range(const char*name,size_t i)const{
            throw std::out_of_range("basic_string_view::"+std::string(name)+": i (which "+std::to_string(i)+") > size() (which is "+std::to_string(len)+")");
        }
    };
    namespace _sv{template<class T,class Traits>using nondeduced_t=std::enable_if_t<true,basic_string_view<T, Traits>>;}
    template<class T, class Traits>constexpr bool
        operator==(basic_string_view<T, Traits> l, basic_string_view<T, Traits> r)
        noexcept{ return l.size() == r.size() && l.compare(r) == 0; }
    template<class T, class Traits>constexpr bool
        operator==(basic_string_view<T, Traits> l, _sv::nondeduced_t<T,Traits> r)
        noexcept { return l.size() == r.size() && l.compare(r) == 0; }
    template<class T, class Traits>constexpr bool
        operator==(_sv::nondeduced_t<T,Traits>l, basic_string_view<T, Traits> r)
        noexcept { return l.size() == r.size() && l.compare(r) == 0; }
    template<class T, class Traits>constexpr bool
        operator!=(basic_string_view<T, Traits>l, basic_string_view<T, Traits> r)
        noexcept{ return !(l == r); }
    template<class T, class Traits>constexpr bool
        operator!=(basic_string_view<T, Traits>l, _sv::nondeduced_t<T,Traits> r)
        noexcept{ return !(l == r); }
    template<class T, class Traits>constexpr bool
        operator!=(_sv::nondeduced_t<T,Traits>l, basic_string_view<T, Traits> r)
        noexcept{ return !(l == r); }

    template<class T, class Traits>constexpr bool
        operator< (basic_string_view<T, Traits>l, basic_string_view<T, Traits> r)
        noexcept{ return l.compare(r) < 0; }
    template<class T, class Traits>constexpr bool
        operator< (basic_string_view<T, Traits>l, _sv::nondeduced_t<T,Traits> r)
        noexcept{ return l.compare(r) < 0; }
    template<class T, class Traits>constexpr bool
        operator< (_sv::nondeduced_t<T,Traits>l, basic_string_view<T, Traits> r)
        noexcept{ return l.compare(r) < 0; }
    template<class T, class Traits>constexpr bool
        operator> (basic_string_view<T, Traits>l, basic_string_view<T, Traits> r)
        noexcept{ return l.compare(r) > 0; }
    template<class T, class Traits>constexpr bool
        operator> (basic_string_view<T, Traits>l, _sv::nondeduced_t<T,Traits> r)
        noexcept{ return l.compare(r) > 0; }
    template<class T, class Traits>constexpr bool
        operator> (_sv::nondeduced_t<T,Traits>l, basic_string_view<T, Traits> r)
        noexcept{ return l.compare(r) > 0; }
    template<class T, class Traits>constexpr bool
        operator<=(basic_string_view<T, Traits>l, basic_string_view<T, Traits> r)
        noexcept{ return l.compare(r) <= 0; }
    template<class T, class Traits>constexpr bool
        operator<=(basic_string_view<T, Traits>l, _sv::nondeduced_t<T,Traits> r)
        noexcept{ return l.compare(r) <= 0; }
    template<class T, class Traits>constexpr bool
        operator<=(_sv::nondeduced_t<T,Traits>l, basic_string_view<T, Traits> r)
        noexcept{ return l.compare(r) <= 0; }
    template<class T, class Traits>constexpr bool
        operator>=(basic_string_view<T, Traits>l, basic_string_view<T, Traits> r)
        noexcept{ return l.compare(r) >= 0; }
    template<class T, class Traits>constexpr bool
        operator>=(basic_string_view<T, Traits>l, _sv::nondeduced_t<T,Traits> r)
        noexcept{ return l.compare(r) >= 0; }
    template<class T, class Traits>constexpr bool
        operator>=(_sv::nondeduced_t<T,Traits>l, basic_string_view<T, Traits> r)
        noexcept{ return l.compare(r) >= 0; }

    using string_view = basic_string_view<char>;
    using wstring_view = basic_string_view<wchar_t>;
#ifdef _GLIBCXX_USE_CHAR8_T
    using u8string_view = basic_string_view<char8_t>;
#endif
    using u16string_view = basic_string_view<char16_t>;
    using u32string_view = basic_string_view<char32_t>;
    template<typename T, typename Traits> inline std::basic_ostream<T, Traits>&
        operator<<(std::basic_ostream<T, Traits>& stream,backports::basic_string_view<T,Traits> str)
        { return stream.write(str.data(),str.size()); }
    namespace _sv{
        template<class T>struct hash{
            typedef size_t result_type;
            typedef backports::basic_string_view<T> argument_type;
            result_type operator()(const argument_type& s) const noexcept {
                //return std::_Hash_bytes(s.data(), s.length()*sizeof(T), static_cast<result_type>(0xc70f6907UL));
                return std::__do_string_hash(s.data(), s.data()+s.length());
                //return std::_Hash_impl::hash(s.data(), s.length()*sizeof(T));
            }
        };
    }
inline namespace literals{inline namespace string_view_literals{
    inline constexpr basic_string_view<char>
    operator""_sv(const char* s, size_t l)
    noexcept{ return {s,l}; }

    inline constexpr basic_string_view<wchar_t>
    operator""_sv(const wchar_t* s, size_t l)
    noexcept{ return {s,l}; }

#ifdef _GLIBCXX_USE_CHAR8_T
    inline constexpr basic_string_view<char8_t>
    operator""_sv(const char8_t* s, size_t l)
    noexcept{ return {s,l}; }
#endif

    inline constexpr basic_string_view<char16_t>
    operator""_sv(const char16_t* s, size_t l)
    noexcept{ return {s,l}; }

    inline constexpr basic_string_view<char32_t>
    operator""_sv(const char32_t* s, size_t l)
    noexcept{ return {s,l}; }
}} // namespace literals::string_literals
} // namespace backports
#include <string_view>
template<>struct std::hash<backports::string_view>:backports::_sv::hash<char>{};
template<>struct std::hash<backports::wstring_view>:backports::_sv::hash<wchar_t>{};
#ifdef _GLIBCXX_USE_CHAR8_T
template<>struct std::hash<backports::u8string_view>:backports::_sv::hash<char8_t>{};
#endif
template<>struct std::hash<backports::u16string_view>:backports::_sv::hash<char16_t>{};
template<>struct std::hash<backports::u32string_view>:backports::_sv::hash<char32_t>{};
#endif // STRING_VIEW_HPP
