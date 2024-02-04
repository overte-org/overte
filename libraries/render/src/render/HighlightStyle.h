//
//  HighlightStyle.h

//  Created by Olivier Prat on 11/06/2017.
//  Copyright 2017 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_render_utils_HighlightStyle_h
#define hifi_render_utils_HighlightStyle_h

#include <glm/vec3.hpp>
#include <glm/gtx/string_cast.hpp>

#include <string>

namespace render {

    // This holds the configuration for a particular outline style
    class HighlightStyle {
    public:
        struct RGBA {
            glm::vec3 color { 1.0f, 0.7f, 0.2f };
            float alpha { 0.9f };

            RGBA(const glm::vec3& c, float a) : color(c), alpha(a) {}

            std::string toString() const { return glm::to_string(color) + " " + std::to_string(alpha); }
        };

        RGBA _outlineUnoccluded { { 1.0f, 0.7f, 0.2f }, 0.9f };
        RGBA _outlineOccluded { { 1.0f, 0.7f, 0.2f }, 0.9f };
        RGBA _fillUnoccluded { { 0.2f, 0.7f, 1.0f }, 0.0f };
        RGBA _fillOccluded { { 0.2f, 0.7f, 1.0f }, 0.0f };

        float _outlineWidth { 2.0f };
        bool _isOutlineSmooth { false };

        bool isFilled() const {
            return _fillUnoccluded.alpha > 5e-3f || _fillOccluded.alpha > 5e-3f;
        }

        std::string toString() const {
            return _outlineUnoccluded.toString() + _outlineOccluded.toString() + _fillUnoccluded.toString() +
                   _fillOccluded.toString() + std::to_string(_outlineWidth) + std::to_string(_isOutlineSmooth);
        }

        static HighlightStyle calculateOutlineStyle(uint8_t mode, float outlineWidth, const glm::vec3& outline,
                const glm::vec3& position, const ViewFrustum& viewFrustum, size_t screenHeight) {
            HighlightStyle style;
            style._outlineUnoccluded.color = outline;
            style._outlineUnoccluded.alpha = 1.0f;
            style._outlineOccluded.alpha = 0.0f;
            style._fillUnoccluded.alpha = 0.0f;
            style._fillOccluded.alpha = 0.0f;
            style._isOutlineSmooth = false;

            if (mode == 1) { // OUTLINE_WORLD
                // FIXME: this is a hacky approximation, which gives us somewhat accurate widths with distance based falloff.
                // Our outline implementation doesn't support the necessary vertex based extrusion to do real world based outlines.
                glm::vec4 viewPos = glm::inverse(viewFrustum.getView()) * glm::vec4(position, 1.0f);

                const glm::mat4& projection = viewFrustum.getProjection();
                glm::vec4 p1 = projection * (viewPos + glm::vec4(0.0f, 0.5f * outlineWidth, 0.0f, 0.0f));
                p1 /= p1.w;
                glm::vec4 p2 = projection * (viewPos - glm::vec4(0.0f, 0.5f * outlineWidth, 0.0f, 0.0f));
                p2 /= p2.w;

                style._outlineWidth = floor(0.5f * (float)screenHeight * fabs(p1.y - p2.y));
            } else { // OUTLINE_SCREEN
                style._outlineWidth = floor(outlineWidth * (float)screenHeight);
            }

            return style;
        }
    };

}

#endif // hifi_render_utils_HighlightStyle_h