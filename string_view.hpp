#ifndef STRING_VIEW_HPP
#define STRING_VIEW_HPP 1
#include <string>
namespace backports{
    template<typename T, typename Traits = std::char_traits<T>>class basic_string_view{
        static_assert(is_trivial_v<T> && is_standard_layout_v<T> && !is_array_v<T>,"Invalid instantiation of basic_string_view");
        static_assert(is_same_v<T, typename Traits::char_type>,"Invalid instantiation of basic_string_view");
        typedef basic_string_view basic_sv;
        size_t len;
        const T* str;
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
        static constexpr size_t npos = size_t(-1);
        constexpr basic_string_view() noexcept:len{0}, str{nullptr}{}
        constexpr basic_string_view(const basic_string_view&) noexcept = default;
        constexpr basic_string_view(const T* s) noexcept :len{traits_type::length(s)},str{s}{}
        constexpr basic_string_view(const T* s, size_t l) noexcept:len{l}, str{s}{}
        constexpr basic_string_view&operator=(const basic_string_view&) noexcept = default;
        constexpr const_iterator begin() const noexcept{ return str; }
        constexpr const_iterator end() const noexcept{ return str + len; }
        constexpr const_iterator cbegin() const noexcept{ return begin(); }
        constexpr const_iterator cend() const noexcept{ return end(); }
        constexpr const_reverse_iterator rbegin() const noexcept{ return const_reverse_iterator(end()); }
        constexpr const_reverse_iterator rend() const noexcept{ return const_reverse_iterator(begin()); }
        constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
        constexpr const_reverse_iterator crend() const noexcept{ return rend(); }
        constexpr size_t size() const noexcept{ return len; }
        constexpr size_t length() const noexcept{ return len; }
        constexpr size_t max_size() const noexcept{
            return (npos - sizeof(size_t) - sizeof(void*))/ sizeof(value_type) / 4;
        }
        constexpr bool empty() const noexcept{ return len == 0; }
        constexpr const_reference operator[](size_t i) const{ return str[i]; }
        constexpr const_reference at(size_t i) const{
            if (i >= len)std::__throw_out_of_range_fmt("basic_string_view::at: i (which is %zu) >= size() (which is %zu)", i, size());
            return str[i];
        }
        constexpr const_reference front() const{ return str[0]; }
        constexpr const_reference back() const{ return str[len-1]; }
        constexpr const_pointer data() const noexcept{ return str; }
        constexpr void remove_prefix(size_t n){ str += n; len -= n; }
        constexpr void remove_suffix(size_t n){ len -= n; }
        constexpr void swap(basic_sv& that) noexcept{ auto tmp = *this;*this = that;that = tmp; }
        constexpr size_t copy(T* that, size_t n, size_t i = 0) const{
            __glibcxx_requires_string_len(that, n);
            if (i > len) std::__throw_out_of_range_fmt("basic_string_view::copy: i (which is %zu) > size() (which is %zu)", i, len);
            n = std::min<size_t>(n, len - i);
            traits_type::copy(that, data() + i, n);
            return n;
        }
        constexpr basic_sv substr(size_t i = 0, size_t n = npos) const{
            if (i > len) std::__throw_out_of_range_fmt("basic_string_view::substr: i (which is %zu) > size() (which is %zu)", i, len);
            n = std::min<size_t>(n, len - i);
            return basic_sv(str + i, n);
        }
        
        constexpr int compare(basic_sv that) const noexcept{
            int r = traits_type::compare(this->str, that.str, std::min(this->len, that.len));
            if (r == 0)r = (this->len>that.len)-(this->len<that.len);
            return r;
        }
        constexpr int compare(size_t i, size_t n, basic_sv that) const{ return substr(i, n).compare(that);}
        constexpr int compare( size_t i1, size_t n1, basic_sv that, size_t i2, size_t n2) const{return substr(i1, n1).compare(that.substr(i2, n2));}
        constexpr int compare(const T* that) const noexcept{ return compare(basic_sv(that)); }
        constexpr int compare(size_t i, size_t n, const T* that) const{return compare(i, n, basic_sv(that));}
        constexpr int compare(size_t i1, size_t n1,const T* that, size_t n2) const {return compare(i1, n1, basic_sv(that, n2));}
        
        constexpr bool starts_with(basic_sv that) const noexcept{ 
            const auto n = that.size();
            return this->size() >= n && traits_type::compare(begin(), that.data(), n) == 0;
        }
        constexpr bool starts_with(T that) const noexcept{ return !empty() && traits_type::eq(front(), that); }
        constexpr bool starts_with(const T* that) const{ return starts_with(basic_sv(that)); }
        
        constexpr bool ends_with(basic_sv that) const noexcept{
            const auto n = that.size();
            return this->size() >= n && traits_type::compare(end() - n, that.data(), n) == 0;
        }
        constexpr bool ends_with(T that) const noexcept{ return !empty() && traits_type::eq(back(), that); }
        constexpr bool ends_with(const T* that) const{ return ends_with(basic_sv(that)); }
        
        constexpr bool contains(basic_sv that) const noexcept { return find(that) != npos; }
        constexpr bool contains(T that) const noexcept{ return find(that) != npos; }
        constexpr bool contains(const T* that) const{ return find(that) != npos; }
        
        constexpr size_t find(basic_sv that, size_t i = 0) const noexcept{
            if (that.empty()) return i <= len ? i : npos;
            const basic_sv lead_finder=substr(0,len-that.size()+1);
            const T lead = that.front();
            for (;i<lead_finder.size();++i){
                i=lead_finder.find(lead,i);
                if (i==npos)return npos;
                if (substr(i).starts_with(that)) return i;
            }
            return npos;
        }
        constexpr size_t find(T that, size_t i = 0) const noexcept{
            if (i >= len){return npos;}
            const T* p = traits_type::find(str + i, len - i, that);
            if (p)return p - str;
            return npos;
        }
        constexpr size_t find(const T* that, size_t i, size_t n) const{ return find(basic_sv(that,n), i); }
        constexpr size_t find(const T* that, size_t i = 0) const { return find(basic_sv(that), i); }
        
        constexpr size_t rfind(basic_sv that, size_t i = npos) const noexcept{
            if(this->len>=that.len)for(i=std::min(this->len-that.len, i)+1;i;)if(substr(--i).starts_with(that))return i;
            return npos;
        }
        constexpr size_t rfind(T that, size_t i = npos) const noexcept{
            for(i=std::min(i+1,len);i;)if(traits_type::eq(str[--i], that))return i;
            return npos;
        }
        constexpr size_t rfind(const T* that, size_t i, size_t n) const{ return find(basic_sv(that,n), i); }
        constexpr size_t rfind(const T* that, size_t i = npos) const { return rfind(basic_sv(that), i); }
        
        constexpr size_t find_first_of(basic_sv that, size_t i = 0) const noexcept{
            if(that.len)for(;i<this->len;++i)if(that.contains(this->str[i]))return i;
            return npos;
        }
        constexpr size_t find_first_of(T that, size_t i = 0) const noexcept{ return find(that, i); }
        constexpr size_t find_first_of(const T* that, size_t i, size_t n) const{ return find(basic_sv(that,n), i); }
        constexpr size_t find_first_of(const T* that, size_t i = 0) const{ return find_first_of(basic_sv(that), i); }
        
        constexpr size_t find_last_of(basic_sv that, size_t i = npos) const noexcept{
            if(that.len)for(i=std::min(this->len,i+1);i;)if(that.contains(this->str[--i]))return i;
            return npos;
        }
        constexpr size_t find_last_of(T that, size_t i=npos) const noexcept{ return rfind(that, i); }
        constexpr size_t find_last_of(const T* that, size_t i, size_t n) const{ return find(basic_sv(that,n), i); }
        constexpr size_t find_last_of(const T* that, size_t i = npos) const{ return find_last_of(basic_sv(that), i); }
        
        constexpr size_t find_first_not_of(basic_sv that, size_t i = 0) const noexcept{
            for (; i < this->len; ++i)if (!that.contains(this->str[i]))return i;
            return npos;
        }
        constexpr size_t find_first_not_of(T that, size_t i = 0) const noexcept{
            for (; i < len; ++i)if (!traits_type::eq(str[i], that))return i;
            return npos;
        }
        constexpr size_t find_first_not_of(const T* that, size_t i, size_t n) const{ return find(basic_sv(that,n), i); }
        constexpr size_t find_first_not_of(const T* that, size_t i = 0) const { return find_first_not_of(basic_sv(that), i); }
        
        constexpr size_t find_last_not_of(basic_sv that, size_t i = npos) const noexcept{
            for(i=std::min(this->len,i+1);i;)if(!that.contains(this->str[--i]))return i;
            return npos;
        }
        constexpr size_t find_last_not_of(T that, size_t i) const noexcept{
            for(i = std::min(len,i+1);i;)if(!traits_type::eq(str[--i], that))return i;
            return npos;
        }
        constexpr size_t find_last_not_of(const T* that, size_t i, size_t n) const{ return find(basic_sv(that,n), i); }   
        constexpr size_t find_last_not_of(const T* that, size_t i = npos) const{ return find_last_not_of(basic_sv(that), i); }
    };
}// namespace backports
#endif // STRING_VIEW_HPP