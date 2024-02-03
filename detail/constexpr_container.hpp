// X-macro array for constexpr containers.
// Arguments:
// copy constructible
// trivially copy constructible
// move constructible
// trivially move constructible
// copy assignable
// trivially copy assignable
// move assignable
// trivially move assignable
// trivially destructible
// copy constructor body
// move constructor body
// coppy assignment body
// move assignment body
// destructor body
// Note: this array assumes assignable objects are always constructible
TEMPLATE(false,false,false,false,false,false,false,false,false,=delete;,=delete;,=delete;,=delete;,{reset();});
TEMPLATE(false,false,false,false,false,false,false,false,true,=delete;,=delete;,=delete;,=delete;,=default;);
TEMPLATE(false,false,true,false,false,false,false,false,false,=delete;,{construct(std::move(that));},=delete;,=delete;,{reset();});
TEMPLATE(false,false,true,false,false,false,false,false,true,=delete;,{construct(std::move(that));},=delete;,=delete;,=default;);
TEMPLATE(false,false,true,false,false,false,true,false,false,=delete;,{construct(std::move(that));},=delete;,{assign(std::move(that));},{reset();});
TEMPLATE(false,false,true,false,false,false,true,false,true,=delete;,{construct(std::move(that));},=delete;,{assign(std::move(that));},=default;);
TEMPLATE(false,false,true,true,false,false,false,false,false,=delete;,=default;,=delete;,=delete;,{reset();});
TEMPLATE(false,false,true,true,false,false,false,false,true,=delete;,=default;,=delete;,=delete;,=default;);
TEMPLATE(false,false,true,true,false,false,true,false,false,=delete;,=default;,=delete;,{assign(std::move(that));},{reset();});
TEMPLATE(false,false,true,true,false,false,true,false,true,=delete;,=default;,=delete;,{assign(std::move(that));},=default;);
TEMPLATE(false,false,true,true,false,false,true,true,false,=delete;,=default;,=delete;,=default;,{reset();});
TEMPLATE(false,false,true,true,false,false,true,true,true,=delete;,=default;,=delete;,=default;,=default;);
TEMPLATE(true,false,false,false,false,false,false,false,false,{construct(that);},=delete;,=delete;,=delete;,{reset();});
TEMPLATE(true,false,false,false,false,false,false,false,true,{construct(that);},=delete;,=delete;,=delete;,=default;);
TEMPLATE(true,false,false,false,true,false,false,false,false,{construct(that);},=delete;,{assign(that);},=delete;,{reset();});
TEMPLATE(true,false,false,false,true,false,false,false,true,{construct(that);},=delete;,{assign(that);},=delete;,=default;);
TEMPLATE(true,false,true,false,false,false,false,false,false,{construct(that);},{construct(std::move(that));},=delete;,=delete;,{reset();});
TEMPLATE(true,false,true,false,false,false,false,false,true,{construct(that);},{construct(std::move(that));},=delete;,=delete;,=default;);
TEMPLATE(true,false,true,false,false,false,true,false,false,{construct(that);},{construct(std::move(that));},=delete;,{assign(std::move(that));},{reset();});
TEMPLATE(true,false,true,false,false,false,true,false,true,{construct(that);},{construct(std::move(that));},=delete;,{assign(std::move(that));},=default;);
TEMPLATE(true,false,true,false,true,false,false,false,false,{construct(that);},{construct(std::move(that));},{assign(that);},=delete;,{reset();});
TEMPLATE(true,false,true,false,true,false,false,false,true,{construct(that);},{construct(std::move(that));},{assign(that);},=delete;,=default;);
TEMPLATE(true,false,true,false,true,false,true,false,false,{construct(that);},{construct(std::move(that));},{assign(that);},{assign(std::move(that));},{reset();});
TEMPLATE(true,false,true,false,true,false,true,false,true,{construct(that);},{construct(std::move(that));},{assign(that);},{assign(std::move(that));},=default;);
TEMPLATE(true,false,true,true,false,false,false,false,false,{construct(that);},=default;,=delete;,=delete;,{reset();});
TEMPLATE(true,false,true,true,false,false,false,false,true,{construct(that);},=default;,=delete;,=delete;,=default;);
TEMPLATE(true,false,true,true,false,false,true,false,false,{construct(that);},=default;,=delete;,{assign(std::move(that));},{reset();});
TEMPLATE(true,false,true,true,false,false,true,false,true,{construct(that);},=default;,=delete;,{assign(std::move(that));},=default;);
TEMPLATE(true,false,true,true,false,false,true,true,false,{construct(that);},=default;,=delete;,=default;,{reset();});
TEMPLATE(true,false,true,true,false,false,true,true,true,{construct(that);},=default;,=delete;,=default;,=default;);
TEMPLATE(true,false,true,true,true,false,false,false,false,{construct(that);},=default;,{assign(that);},=delete;,{reset();});
TEMPLATE(true,false,true,true,true,false,false,false,true,{construct(that);},=default;,{assign(that);},=delete;,=default;);
TEMPLATE(true,false,true,true,true,false,true,false,false,{construct(that);},=default;,{assign(that);},{assign(std::move(that));},{reset();});
TEMPLATE(true,false,true,true,true,false,true,false,true,{construct(that);},=default;,{assign(that);},{assign(std::move(that));},=default;);
TEMPLATE(true,false,true,true,true,false,true,true,false,{construct(that);},=default;,{assign(that);},=default;,{reset();});
TEMPLATE(true,false,true,true,true,false,true,true,true,{construct(that);},=default;,{assign(that);},=default;,=default;);
TEMPLATE(true,true,false,false,false,false,false,false,false,=default;,=delete;,=delete;,=delete;,{reset();});
TEMPLATE(true,true,false,false,false,false,false,false,true,=default;,=delete;,=delete;,=delete;,=default;);
TEMPLATE(true,true,false,false,true,false,false,false,false,=default;,=delete;,{assign(that);},=delete;,{reset();});
TEMPLATE(true,true,false,false,true,false,false,false,true,=default;,=delete;,{assign(that);},=delete;,=default;);
TEMPLATE(true,true,false,false,true,true,false,false,false,=default;,=delete;,=default;,=delete;,{reset();});
TEMPLATE(true,true,false,false,true,true,false,false,true,=default;,=delete;,=default;,=delete;,=default;);
TEMPLATE(true,true,true,false,false,false,false,false,false,=default;,{construct(std::move(that));},=delete;,=delete;,{reset();});
TEMPLATE(true,true,true,false,false,false,false,false,true,=default;,{construct(std::move(that));},=delete;,=delete;,=default;);
TEMPLATE(true,true,true,false,false,false,true,false,false,=default;,{construct(std::move(that));},=delete;,{assign(std::move(that));},{reset();});
TEMPLATE(true,true,true,false,false,false,true,false,true,=default;,{construct(std::move(that));},=delete;,{assign(std::move(that));},=default;);
TEMPLATE(true,true,true,false,true,false,false,false,false,=default;,{construct(std::move(that));},{assign(that);},=delete;,{reset();});
TEMPLATE(true,true,true,false,true,false,false,false,true,=default;,{construct(std::move(that));},{assign(that);},=delete;,=default;);
TEMPLATE(true,true,true,false,true,false,true,false,false,=default;,{construct(std::move(that));},{assign(that);},{assign(std::move(that));},{reset();});
TEMPLATE(true,true,true,false,true,false,true,false,true,=default;,{construct(std::move(that));},{assign(that);},{assign(std::move(that));},=default;);
TEMPLATE(true,true,true,false,true,true,false,false,false,=default;,{construct(std::move(that));},=default;,=delete;,{reset();});
TEMPLATE(true,true,true,false,true,true,false,false,true,=default;,{construct(std::move(that));},=default;,=delete;,=default;);
TEMPLATE(true,true,true,false,true,true,true,false,false,=default;,{construct(std::move(that));},=default;,{assign(std::move(that));},{reset();});
TEMPLATE(true,true,true,false,true,true,true,false,true,=default;,{construct(std::move(that));},=default;,{assign(std::move(that));},=default;);
TEMPLATE(true,true,true,true,false,false,false,false,false,=default;,=default;,=delete;,=delete;,{reset();});
TEMPLATE(true,true,true,true,false,false,false,false,true,=default;,=default;,=delete;,=delete;,=default;);
TEMPLATE(true,true,true,true,false,false,true,false,false,=default;,=default;,=delete;,{assign(std::move(that));},{reset();});
TEMPLATE(true,true,true,true,false,false,true,false,true,=default;,=default;,=delete;,{assign(std::move(that));},=default;);
TEMPLATE(true,true,true,true,false,false,true,true,false,=default;,=default;,=delete;,=default;,{reset();});
TEMPLATE(true,true,true,true,false,false,true,true,true,=default;,=default;,=delete;,=default;,=default;);
TEMPLATE(true,true,true,true,true,false,false,false,false,=default;,=default;,{assign(that);},=delete;,{reset();});
TEMPLATE(true,true,true,true,true,false,false,false,true,=default;,=default;,{assign(that);},=delete;,=default;);
TEMPLATE(true,true,true,true,true,false,true,false,false,=default;,=default;,{assign(that);},{assign(std::move(that));},{reset();});
TEMPLATE(true,true,true,true,true,false,true,false,true,=default;,=default;,{assign(that);},{assign(std::move(that));},=default;);
TEMPLATE(true,true,true,true,true,false,true,true,false,=default;,=default;,{assign(that);},=default;,{reset();});
TEMPLATE(true,true,true,true,true,false,true,true,true,=default;,=default;,{assign(that);},=default;,=default;);
TEMPLATE(true,true,true,true,true,true,false,false,false,=default;,=default;,=default;,=delete;,{reset();});
TEMPLATE(true,true,true,true,true,true,false,false,true,=default;,=default;,=default;,=delete;,=default;);
TEMPLATE(true,true,true,true,true,true,true,false,false,=default;,=default;,=default;,{assign(std::move(that));},{reset();});
TEMPLATE(true,true,true,true,true,true,true,false,true,=default;,=default;,=default;,{assign(std::move(that));},=default;);
TEMPLATE(true,true,true,true,true,true,true,true,false,=default;,=default;,=default;,=default;,{reset();});
TEMPLATE(true,true,true,true,true,true,true,true,true,=default;,=default;,=default;,=default;,=default;);
