#include <iostream>
#include <string>
#include <regex>
#include <map>

extern "C" {
    #include <stdint.h>
}

const std::map<std::string, std::tuple<float, float, float, float>> COLORS = {
    {"aliceblue", std::make_tuple(240 / 255.0f, 248 / 255.0f, 255 / 255.0f, 1)},
    {"antiquewhite", std::make_tuple(250 / 255.0f, 235 / 255.0f, 215 / 255.0f, 1)},
    {"aqua", std::make_tuple(0 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1)},
    {"aquamarine", std::make_tuple(127 / 255.0f, 255 / 255.0f, 212 / 255.0f, 1)},
    {"azure", std::make_tuple(240 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1)},
    {"beige", std::make_tuple(245 / 255.0f, 245 / 255.0f, 220 / 255.0f, 1)},
    {"bisque", std::make_tuple(255 / 255.0f, 228 / 255.0f, 196 / 255.0f, 1)},
    {"black", std::make_tuple(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1)},
    {"blanchedalmond", std::make_tuple(255 / 255.0f, 235 / 255.0f, 205 / 255.0f, 1)},
    {"blue", std::make_tuple(0 / 255.0f, 0 / 255.0f, 255 / 255.0f, 1)},
    {"blueviolet", std::make_tuple(138 / 255.0f, 43 / 255.0f, 226 / 255.0f, 1)},
    {"brown", std::make_tuple(165 / 255.0f, 42 / 255.0f, 42 / 255.0f, 1)},
    {"burlywood", std::make_tuple(222 / 255.0f, 184 / 255.0f, 135 / 255.0f, 1)},
    {"cadetblue", std::make_tuple(95 / 255.0f, 158 / 255.0f, 160 / 255.0f, 1)},
    {"chartreuse", std::make_tuple(127 / 255.0f, 255 / 255.0f, 0 / 255.0f, 1)},
    {"chocolate", std::make_tuple(210 / 255.0f, 105 / 255.0f, 30 / 255.0f, 1)},
    {"coral", std::make_tuple(255 / 255.0f, 127 / 255.0f, 80 / 255.0f, 1)},
    {"cornflowerblue", std::make_tuple(100 / 255.0f, 149 / 255.0f, 237 / 255.0f, 1)},
    {"cornsilk", std::make_tuple(255 / 255.0f, 248 / 255.0f, 220 / 255.0f, 1)},
    {"crimson", std::make_tuple(220 / 255.0f, 20 / 255.0f, 60 / 255.0f, 1)},
    {"cyan", std::make_tuple(0 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1)},
    {"darkblue", std::make_tuple(0 / 255.0f, 0 / 255.0f, 139 / 255.0f, 1)},
    {"darkcyan", std::make_tuple(0 / 255.0f, 139 / 255.0f, 139 / 255.0f, 1)},
    {"darkgoldenrod", std::make_tuple(184 / 255.0f, 134 / 255.0f, 11 / 255.0f, 1)},
    {"darkgray", std::make_tuple(169 / 255.0f, 169 / 255.0f, 169 / 255.0f, 1)},
    {"darkgreen", std::make_tuple(0 / 255.0f, 100 / 255.0f, 0 / 255.0f, 1)},
    {"darkgrey", std::make_tuple(169 / 255.0f, 169 / 255.0f, 169 / 255.0f, 1)},
    {"darkkhaki", std::make_tuple(189 / 255.0f, 183 / 255.0f, 107 / 255.0f, 1)},
    {"darkmagenta", std::make_tuple(139 / 255.0f, 0 / 255.0f, 139 / 255.0f, 1)},
    {"darkolivegreen", std::make_tuple(85 / 255.0f, 107 / 255.0f, 47 / 255.0f, 1)},
    {"darkorange", std::make_tuple(255 / 255.0f, 140 / 255.0f, 0 / 255.0f, 1)},
    {"darkorchid", std::make_tuple(153 / 255.0f, 50 / 255.0f, 204 / 255.0f, 1)},
    {"darkred", std::make_tuple(139 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1)},
    {"darksalmon", std::make_tuple(233 / 255.0f, 150 / 255.0f, 122 / 255.0f, 1)},
    {"darkseagreen", std::make_tuple(143 / 255.0f, 188 / 255.0f, 143 / 255.0f, 1)},
    {"darkslateblue", std::make_tuple(72 / 255.0f, 61 / 255.0f, 139 / 255.0f, 1)},
    {"darkslategray", std::make_tuple(47 / 255.0f, 79 / 255.0f, 79 / 255.0f, 1)},
    {"darkslategrey", std::make_tuple(47 / 255.0f, 79 / 255.0f, 79 / 255.0f, 1)},
    {"darkturquoise", std::make_tuple(0 / 255.0f, 206 / 255.0f, 209 / 255.0f, 1)},
    {"darkviolet", std::make_tuple(148 / 255.0f, 0 / 255.0f, 211 / 255.0f, 1)},
    {"deeppink", std::make_tuple(255 / 255.0f, 20 / 255.0f, 147 / 255.0f, 1)},
    {"deepskyblue", std::make_tuple(0 / 255.0f, 191 / 255.0f, 255 / 255.0f, 1)},
    {"dimgray", std::make_tuple(105 / 255.0f, 105 / 255.0f, 105 / 255.0f, 1)},
    {"dimgrey", std::make_tuple(105 / 255.0f, 105 / 255.0f, 105 / 255.0f, 1)},
    {"dodgerblue", std::make_tuple(30 / 255.0f, 144 / 255.0f, 255 / 255.0f, 1)},
    {"firebrick", std::make_tuple(178 / 255.0f, 34 / 255.0f, 34 / 255.0f, 1)},
    {"floralwhite", std::make_tuple(255 / 255.0f, 250 / 255.0f, 240 / 255.0f, 1)},
    {"forestgreen", std::make_tuple(34 / 255.0f, 139 / 255.0f, 34 / 255.0f, 1)},
    {"fuchsia", std::make_tuple(255 / 255.0f, 0 / 255.0f, 255 / 255.0f, 1)},
    {"gainsboro", std::make_tuple(220 / 255.0f, 220 / 255.0f, 220 / 255.0f, 1)},
    {"ghostwhite", std::make_tuple(248 / 255.0f, 248 / 255.0f, 255 / 255.0f, 1)},
    {"gold", std::make_tuple(255 / 255.0f, 215 / 255.0f, 0 / 255.0f, 1)},
    {"goldenrod", std::make_tuple(218 / 255.0f, 165 / 255.0f, 32 / 255.0f, 1)},
    {"gray", std::make_tuple(128 / 255.0f, 128 / 255.0f, 128 / 255.0f, 1)},
    {"grey", std::make_tuple(128 / 255.0f, 128 / 255.0f, 128 / 255.0f, 1)},
    {"green", std::make_tuple(0 / 255.0f, 128 / 255.0f, 0 / 255.0f, 1)},
    {"greenyellow", std::make_tuple(173 / 255.0f, 255 / 255.0f, 47 / 255.0f, 1)},
    {"honeydew", std::make_tuple(240 / 255.0f, 255 / 255.0f, 240 / 255.0f, 1)},
    {"hotpink", std::make_tuple(255 / 255.0f, 105 / 255.0f, 180 / 255.0f, 1)},
    {"indianred", std::make_tuple(205 / 255.0f, 92 / 255.0f, 92 / 255.0f, 1)},
    {"indigo", std::make_tuple(75 / 255.0f, 0 / 255.0f, 130 / 255.0f, 1)},
    {"ivory", std::make_tuple(255 / 255.0f, 255 / 255.0f, 240 / 255.0f, 1)},
    {"khaki", std::make_tuple(240 / 255.0f, 230 / 255.0f, 140 / 255.0f, 1)},
    {"lavender", std::make_tuple(230 / 255.0f, 230 / 255.0f, 250 / 255.0f, 1)},
    {"lavenderblush", std::make_tuple(255 / 255.0f, 240 / 255.0f, 245 / 255.0f, 1)},
    {"lawngreen", std::make_tuple(124 / 255.0f, 252 / 255.0f, 0 / 255.0f, 1)},
    {"lemonchiffon", std::make_tuple(255 / 255.0f, 250 / 255.0f, 205 / 255.0f, 1)},
    {"lightblue", std::make_tuple(173 / 255.0f, 216 / 255.0f, 230 / 255.0f, 1)},
    {"lightcoral", std::make_tuple(240 / 255.0f, 128 / 255.0f, 128 / 255.0f, 1)},
    {"lightcyan", std::make_tuple(224 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1)},
    {"lightgoldenrodyellow", std::make_tuple(250 / 255.0f, 250 / 255.0f, 210 / 255.0f, 1)},
    {"lightgray", std::make_tuple(211 / 255.0f, 211 / 255.0f, 211 / 255.0f, 1)},
    {"lightgreen", std::make_tuple(144 / 255.0f, 238 / 255.0f, 144 / 255.0f, 1)},
    {"lightgrey", std::make_tuple(211 / 255.0f, 211 / 255.0f, 211 / 255.0f, 1)},
    {"lightpink", std::make_tuple(255 / 255.0f, 182 / 255.0f, 193 / 255.0f, 1)},
    {"lightsalmon", std::make_tuple(255 / 255.0f, 160 / 255.0f, 122 / 255.0f, 1)},
    {"lightseagreen", std::make_tuple(32 / 255.0f, 178 / 255.0f, 170 / 255.0f, 1)},
    {"lightskyblue", std::make_tuple(135 / 255.0f, 206 / 255.0f, 250 / 255.0f, 1)},
    {"lightslategray", std::make_tuple(119 / 255.0f, 136 / 255.0f, 153 / 255.0f, 1)},
    {"lightslategrey", std::make_tuple(119 / 255.0f, 136 / 255.0f, 153 / 255.0f, 1)},
    {"lightsteelblue", std::make_tuple(176 / 255.0f, 196 / 255.0f, 222 / 255.0f, 1)},
    {"lightyellow", std::make_tuple(255 / 255.0f, 255 / 255.0f, 224 / 255.0f, 1)},
    {"lime", std::make_tuple(0 / 255.0f, 255 / 255.0f, 0 / 255.0f, 1)},
    {"limegreen", std::make_tuple(50 / 255.0f, 205 / 255.0f, 50 / 255.0f, 1)},
    {"linen", std::make_tuple(250 / 255.0f, 240 / 255.0f, 230 / 255.0f, 1)},
    {"magenta", std::make_tuple(255 / 255.0f, 0 / 255.0f, 255 / 255.0f, 1)},
    {"maroon", std::make_tuple(128 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1)},
    {"mediumaquamarine", std::make_tuple(102 / 255.0f, 205 / 255.0f, 170 / 255.0f, 1)},
    {"mediumblue", std::make_tuple(0 / 255.0f, 0 / 255.0f, 205 / 255.0f, 1)},
    {"mediumorchid", std::make_tuple(186 / 255.0f, 85 / 255.0f, 211 / 255.0f, 1)},
    {"mediumpurple", std::make_tuple(147 / 255.0f, 112 / 255.0f, 219 / 255.0f, 1)},
    {"mediumseagreen", std::make_tuple(60 / 255.0f, 179 / 255.0f, 113 / 255.0f, 1)},
    {"mediumslateblue", std::make_tuple(123 / 255.0f, 104 / 255.0f, 238 / 255.0f, 1)},
    {"mediumspringgreen", std::make_tuple(0 / 255.0f, 250 / 255.0f, 154 / 255.0f, 1)},
    {"mediumturquoise", std::make_tuple(72 / 255.0f, 209 / 255.0f, 204 / 255.0f, 1)},
    {"mediumvioletred", std::make_tuple(199 / 255.0f, 21 / 255.0f, 133 / 255.0f, 1)},
    {"midnightblue", std::make_tuple(25 / 255.0f, 25 / 255.0f, 112 / 255.0f, 1)},
    {"mintcream", std::make_tuple(245 / 255.0f, 255 / 255.0f, 250 / 255.0f, 1)},
    {"mistyrose", std::make_tuple(255 / 255.0f, 228 / 255.0f, 225 / 255.0f, 1)},
    {"moccasin", std::make_tuple(255 / 255.0f, 228 / 255.0f, 181 / 255.0f, 1)},
    {"navajowhite", std::make_tuple(255 / 255.0f, 222 / 255.0f, 173 / 255.0f, 1)},
    {"navy", std::make_tuple(0 / 255.0f, 0 / 255.0f, 128 / 255.0f, 1)},
    {"oldlace", std::make_tuple(253 / 255.0f, 245 / 255.0f, 230 / 255.0f, 1)},
    {"olive", std::make_tuple(128 / 255.0f, 128 / 255.0f, 0 / 255.0f, 1)},
    {"olivedrab", std::make_tuple(107 / 255.0f, 142 / 255.0f, 35 / 255.0f, 1)},
    {"orange", std::make_tuple(255 / 255.0f, 165 / 255.0f, 0 / 255.0f, 1)},
    {"orangered", std::make_tuple(255 / 255.0f, 69 / 255.0f, 0 / 255.0f, 1)},
    {"orchid", std::make_tuple(218 / 255.0f, 112 / 255.0f, 214 / 255.0f, 1)},
    {"palegoldenrod", std::make_tuple(238 / 255.0f, 232 / 255.0f, 170 / 255.0f, 1)},
    {"palegreen", std::make_tuple(152 / 255.0f, 251 / 255.0f, 152 / 255.0f, 1)},
    {"paleturquoise", std::make_tuple(175 / 255.0f, 238 / 255.0f, 238 / 255.0f, 1)},
    {"palevioletred", std::make_tuple(219 / 255.0f, 112 / 255.0f, 147 / 255.0f, 1)},
    {"papayawhip", std::make_tuple(255 / 255.0f, 239 / 255.0f, 213 / 255.0f, 1)},
    {"peachpuff", std::make_tuple(255 / 255.0f, 218 / 255.0f, 185 / 255.0f, 1)},
    {"peru", std::make_tuple(205 / 255.0f, 133 / 255.0f, 63 / 255.0f, 1)},
    {"pink", std::make_tuple(255 / 255.0f, 192 / 255.0f, 203 / 255.0f, 1)},
    {"plum", std::make_tuple(221 / 255.0f, 160 / 255.0f, 221 / 255.0f, 1)},
    {"powderblue", std::make_tuple(176 / 255.0f, 224 / 255.0f, 230 / 255.0f, 1)},
    {"purple", std::make_tuple(128 / 255.0f, 0 / 255.0f, 128 / 255.0f, 1)},
    {"red", std::make_tuple(255 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1)},
    {"rosybrown", std::make_tuple(188 / 255.0f, 143 / 255.0f, 143 / 255.0f, 1)},
    {"royalblue", std::make_tuple(65 / 255.0f, 105 / 255.0f, 225 / 255.0f, 1)},
    {"saddlebrown", std::make_tuple(139 / 255.0f, 69 / 255.0f, 19 / 255.0f, 1)},
    {"salmon", std::make_tuple(250 / 255.0f, 128 / 255.0f, 114 / 255.0f, 1)},
    {"sandybrown", std::make_tuple(244 / 255.0f, 164 / 255.0f, 96 / 255.0f, 1)},
    {"seagreen", std::make_tuple(46 / 255.0f, 139 / 255.0f, 87 / 255.0f, 1)},
    {"seashell", std::make_tuple(255 / 255.0f, 245 / 255.0f, 238 / 255.0f, 1)},
    {"sienna", std::make_tuple(160 / 255.0f, 82 / 255.0f, 45 / 255.0f, 1)},
    {"silver", std::make_tuple(192 / 255.0f, 192 / 255.0f, 192 / 255.0f, 1)},
    {"skyblue", std::make_tuple(135 / 255.0f, 206 / 255.0f, 235 / 255.0f, 1)},
    {"slateblue", std::make_tuple(106 / 255.0f, 90 / 255.0f, 205 / 255.0f, 1)},
    {"slategray", std::make_tuple(112 / 255.0f, 128 / 255.0f, 144 / 255.0f, 1)},
    {"slategrey", std::make_tuple(112 / 255.0f, 128 / 255.0f, 144 / 255.0f, 1)},
    {"snow", std::make_tuple(255 / 255.0f, 250 / 255.0f, 250 / 255.0f, 1)},
    {"springgreen", std::make_tuple(0 / 255.0f, 255 / 255.0f, 127 / 255.0f, 1)},
    {"steelblue", std::make_tuple(70 / 255.0f, 130 / 255.0f, 180 / 255.0f, 1)},
    {"tan", std::make_tuple(210 / 255.0f, 180 / 255.0f, 140 / 255.0f, 1)},
    {"teal", std::make_tuple(0 / 255.0f, 128 / 255.0f, 128 / 255.0f, 1)},
    {"thistle", std::make_tuple(216 / 255.0f, 191 / 255.0f, 216 / 255.0f, 1)},
    {"tomato", std::make_tuple(255 / 255.0f, 99 / 255.0f, 71 / 255.0f, 1)},
    {"turquoise", std::make_tuple(64 / 255.0f, 224 / 255.0f, 208 / 255.0f, 1)},
    {"violet", std::make_tuple(238 / 255.0f, 130 / 255.0f, 238 / 255.0f, 1)},
    {"wheat", std::make_tuple(245 / 255.0f, 222 / 255.0f, 179 / 255.0f, 1)},
    {"white", std::make_tuple(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1)},
    {"whitesmoke", std::make_tuple(245 / 255.0f, 245 / 255.0f, 245 / 255.0f, 1)},
    {"yellow", std::make_tuple(255 / 255.0f, 255 / 255.0f, 0 / 255.0f, 1)},
    {"yellowgreen", std::make_tuple(154 / 255.0f, 205 / 255.0f, 50 / 255.0f, 1)},
    
    {"activeborder", std::make_tuple(0, 0, 1, 1)},
    {"activecaption", std::make_tuple(0, 0, 1, 1)},
    {"appworkspace", std::make_tuple(1, 1, 1, 1)},
    {"background", std::make_tuple(1, 1, 1, 1)},
    {"buttonface", std::make_tuple(0, 0, 0, 1)},
    {"buttonhighlight", std::make_tuple(0.8f, 0.8f, 0.8f, 1)},
    {"buttonshadow", std::make_tuple(0.2f, 0.2f, 0.2f, 1)},
    {"buttontext", std::make_tuple(0, 0, 0, 1)},
    {"captiontext", std::make_tuple(0, 0, 0, 1)},
    {"graytext", std::make_tuple(0.2f, 0.2f, 0.2f, 1)},
    {"highlight", std::make_tuple(0, 0, 1, 1)},
    {"highlighttext", std::make_tuple(0.8f, 0.8f, 0.8f, 1)},
    {"inactiveborder", std::make_tuple(0.2f, 0.2f, 0.2f, 1)},
    {"inactivecaption", std::make_tuple(0.8f, 0.8f, 0.8f, 1)},
    {"inactivecaptiontext", std::make_tuple(0.2f, 0.2f, 0.2f, 1)},
    {"infobackground", std::make_tuple(0.8f, 0.8f, 0.8f, 1)},
    {"infotext", std::make_tuple(0, 0, 0, 1)},
    {"menu", std::make_tuple(0.8f, 0.8f, 0.8f, 1)},
    {"menutext", std::make_tuple(0.2f, 0.2f, 0.2f, 1)},
    {"scrollbar", std::make_tuple(0.8f, 0.8f, 0.8f, 1)},
    {"threeddarkshadow", std::make_tuple(0.2f, 0.2f, 0.2f, 1)},
    {"threedface", std::make_tuple(0.8f, 0.8f, 0.8f, 1)},
    {"threedhighlight", std::make_tuple(1, 1, 1, 1)},
    {"threedlightshadow", std::make_tuple(0.2f, 0.2f, 0.2f, 1)},
    {"threedshadow", std::make_tuple(0.2f, 0.2f, 0.2f, 1)},
    {"window", std::make_tuple(0.8f, 0.8f, 0.8f, 1)},
    {"windowframe", std::make_tuple(0.8f, 0.8f, 0.8f, 1)},
    {"windowtext", std::make_tuple(0, 0, 0, 1)},

    {"none", std::make_tuple(0, 0, 0, 0)},
    {"transparent", std::make_tuple(0, 0, 0, 0)}
};

std::regex RGBA(R"(rgba\((.+?)\))");
std::regex RGB(R"(rgb\((.+?)\))");
std::regex HEX_RRGGBB(R"(#[0-9a-f]{6})");
std::regex HEX_RGB(R"(#[0-9a-f]{3})");

std::tuple<float, float, float, float> color(const std::string& string, float opacity = 1.0f) {
    if (string.empty()) {
        return std::make_tuple(0, 0, 0, 0);
    }

    std::string str = string;
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    str.erase(str.find_last_not_of(" \n\r\t") + 1);

    if (COLORS.find(str) != COLORS.end()) {
        auto [r, g, b, a] = COLORS.at(str);
        return std::make_tuple(r, g, b, a * opacity);
    }

    std::smatch match;
    if (std::regex_search(str, match, RGBA)) {
        std::string group = match[1];
        float r, g, b, a;
        sscanf(group.c_str(), "%f,%f,%f,%f", &r, &g, &b, &a);
        return std::make_tuple(r / 255, g / 255, b / 255, a * opacity);
    }

    if (std::regex_search(str, match, RGB)) {
        std::string group = match[1];
        float r, g, b;
        sscanf(group.c_str(), "%f,%f,%f", &r, &g, &b);
        return std::make_tuple(r / 255, g / 255, b / 255, opacity);
    }

    if (std::regex_search(str, match, HEX_RRGGBB)) {
        unsigned int r, g, b;
        sscanf(str.c_str(), "#%02x%02x%02x", &r, &g, &b);
        return std::make_tuple(r / 255.0f, g / 255.0f, b / 255.0f, opacity);
    }

    if (std::regex_search(str, match, HEX_RGB)) {
        unsigned int r, g, b;
        sscanf(str.c_str(), "#%1x%1x%1x", &r, &g, &b);
        return std::make_tuple(r / 15.0f, g / 15.0f, b / 15.0f, opacity);
    }

    return std::make_tuple(0, 0, 0, 1);
}

std::tuple<float, float, float, float> negate_color(std::tuple<float, float, float, float> rgba_tuple) {
    auto [r, g, b, a] = rgba_tuple;
    return std::make_tuple(1 - r, 1 - g, 1 - b, a);
}

int main() {
    std::string colorString = "rgba(255, 0, 0, 0.5)";
    auto rgba = color(colorString, 0.5);
    auto negated = negate_color(rgba);

    std::cout << "Original Color: (" << std::get<0>(rgba) << ", " << std::get<1>(rgba) << ", " << std::get<2>(rgba) << ", " << std::get<3>(rgba) << ")\n";
    std::cout << "Negated Color: (" << std::get<0>(negated) << ", " << std::get<1>(negated) << ", " << std::get<2>(negated) << ", " << std::get<3>(negated) << ")\n";

    return 0;
}