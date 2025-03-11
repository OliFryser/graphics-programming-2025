Shader "CG2024/CelShading"
{
    Properties
    {
        _Albedo("Albedo", Color) = (1,1,1,1)
        _AlbedoTexture("Albedo Texture", 2D) = "white" {}
        _Reflectance("Reflectance (Ambient, Diffuse, Specular)", Vector) = (1, 1, 1, 0)
        _SpecularExponent("Specular Exponent", Float) = 100.0
        _Levels("Levels", Int) = 3
    }

    SubShader
    {
        Tags { "RenderType" = "Opaque" }

        GLSLINCLUDE
        #include "UnityCG.glslinc"
        #include "ITUCG.glslinc"

        uniform vec4 _Albedo;
        uniform sampler2D _AlbedoTexture;
		// Albedo texture coordinate scale and offset
        uniform vec4 _AlbedoTexture_ST;
        uniform vec4 _Reflectance;
        uniform float _SpecularExponent;
        uniform int _Levels;
        ENDGLSL

        Pass
        {
            Name "FORWARD"
            Tags { "LightMode" = "ForwardBase" }

            GLSLPROGRAM

            struct vertexToFragment
            {
                vec3 worldPos;
                vec3 normal;
                vec2 texCoords;
            };

            #ifdef VERTEX
            out vertexToFragment v2f;

            void main()
            {
                // Transform position and normal to world space
                v2f.worldPos = (unity_ObjectToWorld * gl_Vertex).xyz;
                v2f.normal = (unity_ObjectToWorld * vec4(gl_Normal, 0.0f)).xyz;

                // Transform texture coordinates with the Scale and Offset provided in the material
                v2f.texCoords = TransformTexCoords(gl_MultiTexCoord0.xy, _AlbedoTexture_ST);

                // Project vertex position
                gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
            }
            #endif // VERTEX

            #ifdef FRAGMENT
            in vertexToFragment v2f;

            void main()
            {
                // Get light and view directions (L and V)
                vec3 lightDir = GetWorldSpaceLightDir(v2f.worldPos);
                vec3 viewDir = GetWorldSpaceViewDir(v2f.worldPos);

                // Normalize normal after linear interpolation
                vec3 normal = normalize(v2f.normal);

                // Sample albedo texture
                vec3 albedo = texture(_AlbedoTexture, v2f.texCoords).rgb;
                albedo *= _Albedo.rgb;

                // Compute lighting
                vec3 lighting = BlinnPhongLighting(lightDir, viewDir, normal, vec3(1.0f), vec3(1.0f), _Reflectance.x, _Reflectance.y, _Reflectance.z, _SpecularExponent);

                // Get intensity
                float intensity = GetColorLuminance(lighting);
                intensity *= _Levels;
                intensity = ceil(intensity) / _Levels;

                gl_FragColor = intensity * vec4(albedo, 1.0f) * _LightColor0;
            }
            #endif // FRAGMENT

            ENDGLSL
        }
        Pass
        {
            Name "FORWARD"
            Tags { "LightMode" = "ForwardAdd" }

            ZWrite Off
            Blend One One

            GLSLPROGRAM

            struct vertexToFragment
            {
                vec3 worldPos;
                vec3 normal;
                vec2 texCoords;
            };

            #ifdef VERTEX
            out vertexToFragment v2f;

            void main()
            {
                v2f.worldPos = (unity_ObjectToWorld * gl_Vertex).xyz;
                v2f.normal = (unity_ObjectToWorld * vec4(gl_Normal, 0.0f)).xyz;
                v2f.texCoords = TransformTexCoords(gl_MultiTexCoord0.xy, _AlbedoTexture_ST);

                gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
            }
            #endif // VERTEX

            #ifdef FRAGMENT
            in vertexToFragment v2f;

            void main()
            {
                vec3 lightDir = GetWorldSpaceLightDir(v2f.worldPos);
                vec3 viewDir = GetWorldSpaceViewDir(v2f.worldPos);

                vec3 normal = normalize(v2f.normal);

                vec3 albedo = texture(_AlbedoTexture, v2f.texCoords).rgb;
                albedo *= _Albedo.rgb;

               // Compute lighting
                vec3 lighting = BlinnPhongLighting(lightDir, viewDir, normal, vec3(1.0f), vec3(1.0f), _Reflectance.x, _Reflectance.y, _Reflectance.z, _SpecularExponent);

                // Get intensity
                float intensity = GetColorLuminance(lighting);
                intensity *= _Levels;
                intensity = ceil(intensity) / _Levels;

                gl_FragColor = intensity * vec4(albedo, 1.0f) * _LightColor0;
            }
            #endif // FRAGMENT

            ENDGLSL
        }
        // ShadowCaster pass. This will be the shader executed to generate a shadow map.
        Pass
        {
            Name "SHADOWCASTER"
            Tags { "LightMode" = "ShadowCaster" }

            GLSLPROGRAM

            #ifdef VERTEX
            void main()
            {
                gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
            }
            #endif // VERTEX

            #ifdef FRAGMENT
            void main()
            {
                // No output, since we are only using the output from depth
            }
            #endif // FRAGMENT

            ENDGLSL
        }

        Pass
        {
            Name "OUTLINE"
            Tags { "LightMode" = "ForwardBase" }

            Cull Front

            GLSLPROGRAM

            #ifdef VERTEX

            void main()
            {
                float outlineSize = 0.02f;
                vec3 worldPos = (unity_ObjectToWorld * gl_Vertex).xyz;
                vec3 normal = (unity_ObjectToWorld * vec4(gl_Normal, 0.0f)).xyz;
                worldPos += normal * outlineSize;
                gl_Position = unity_MatrixVP * vec4(worldPos, 1.0);
            }
            #endif // VERTEX

            #ifdef FRAGMENT

            void main()
            {
               gl_FragColor = vec4(vec3(0.0f), 1.0f);
            }
            #endif // FRAGMENT

            ENDGLSL
        }
    }
}
