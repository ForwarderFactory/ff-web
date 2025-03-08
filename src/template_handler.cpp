#include <bygg/bygg.hpp>
#include <ff.hpp>
#include <string>

bygg::HTML::Section ff::get_grid(const std::vector<bygg::HTML::Section>& sections, const std::string& classes, const std::string& id) {
    using namespace bygg::HTML;

    std::string _classes{"grid"};
    if (classes.empty() == false) {
        _classes += " " + classes;
    }

    Properties props;
    props.push_back(Property{"class", _classes});

    if (!id.empty()) {
        props.push_back(Property{"id", id});
    }

    Section grid{Tag::Div, props};

    for (const auto& it : sections) {
        Section s = it;
        auto properties = s.get_properties();
        if (properties.find("class") != Properties::npos) {
            properties.at(properties.find("class")) = Property{"class", "grid-item " + properties.at(properties.find("class")).get_value()};
        } else {
            properties += Property{"class", "grid-item"};
        }
        s.set_properties(properties);
        grid += s;
    }

    return grid;
}

bygg::HTML::Section ff::get_link_box(const LinkBoxProperties& p) {
    using namespace bygg::HTML;
    std::string classes = "link_box";
    if (p.classes.empty() == false) {
        classes += " " + p.classes;
    }
    Properties props;
    props.push_back(Property{"class", classes});

    if (p.location.empty() == false) {
        props.push_back(Property{"onclick", "location.href='" + p.location + "';"});
    } else if (p.onclick.empty() == false) {
        props.push_back(Property{"onclick", p.onclick});
    }

    if (p.id.empty() == false) {
        props.push_back(Property{"id", p.id});
    }

    if (p.background_color.empty() == false || p.color.empty() == false) {
        std::string style;
        if (p.background_color.empty() == false) {
            style += "background-color: " + p.background_color + ";";
        }
        if (p.color.empty() == false) {
            style += "color: " + p.color + ";";
        }
        props.push_back(Property{"style", style});
    }

    return Section{Tag::Div, props,
        Element{Tag::H2, Property{"class", "link_box_title"}, p.title},
        Element{Tag::P, Property{"class", "link_box_description"}, p.description},
    };
}

bygg::HTML::Section ff::get_site_head(const limhamn::http::server::request& req, const SiteProperties& properties) {
    using namespace bygg::HTML;
    return Section{Tag::Head,
        Element{Tag::Title, properties.title},
        Element{Tag::Meta, make_properties(Property{"charset", "utf-8"})},
        Element{Tag::Meta, make_properties(Property{"name", "viewport"}, Property{"content", "width=device-width, initial-scale=1"})},
        Element{Tag::Meta, make_properties(Property{"name", "description"}, Property{"content", properties.description})},
        Element{Tag::Meta, make_properties(Property{"name", "author"}, Property{"content", "Forwarder Factory"})},
        Element{Tag::Meta, make_properties(Property{"name", "theme-color"}, Property{"content", "#00a8ff"})},
        Element{Tag::Meta, make_properties(Property{"name", "keywords"}, Property{"content", "Wii, Wii U, Forwarder Factory, Modding, Software, Development, Nintendo, Hacking, Download"})},
        Element{Tag::Meta, make_properties(Property{"property", "og:title"}, Property{"content", properties.title})},
        Element{Tag::Meta, make_properties(Property{"property", "og:description"}, Property{"content", properties.description})},
        Element{Tag::Meta, make_properties(Property{"property", "og:type"}, Property{"content", "website"})},
        Element{Tag::Meta, make_properties(Property{"property", "og:locale"}, Property{"content", "en_US"})},
        Element{Tag::Meta, make_properties(Property{"property", "og:site_name"}, Property{"content", "forwarderfactory.com"})},
        Element{Tag::Meta, make_properties(Property{"property", "og:url"}, Property{"content", "forwarderfactory.com"})},
        Element{Tag::Link, make_properties(Property{"href", properties.css_path}, Property{"rel", "stylesheet"})},
        Element{Tag::Link, make_properties(Property{"rel", "icon"}, Property{"type", "image/x-icon"}, Property{"href", "/img/favicon.ico"})},
        Element{Tag::Script, make_properties(Property{"src", properties.js_path})},
    };
}