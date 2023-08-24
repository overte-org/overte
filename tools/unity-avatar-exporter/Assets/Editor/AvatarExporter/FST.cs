


using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;
using UnityEditor;
using UnityEngine;
using System.Globalization;

namespace Overte
{
    class JointMap
    {
        public string From;
        public string To;

        private Regex parseRx = new Regex(@"^(?<From>[\w]*)\s*=\s*(?<To>.*)", RegexOptions.Compiled | RegexOptions.IgnoreCase);

        public JointMap(string RawInput)
        {
            var parsed = parseRx.Matches(RawInput).First();
            From = parsed.Groups["From"].Value.Trim();
            To = parsed.Groups["To"].Value.Trim();
        }

        public JointMap(string f, string t)
        {
            From = f; To = t;
        }

        public override string ToString() => $"jointMap = {From} = {To}";
    }
    
    class Joint
    {
        public string From;
        public string To;

        private Regex parseRx = new Regex(@"^(?<From>[\w]*)\s*=\s*(?<To>.*)", RegexOptions.Compiled | RegexOptions.IgnoreCase);

        public Joint(string RawInput)
        {
            var parsed = parseRx.Matches(RawInput).First();
            From = parsed.Groups["From"].Value.Trim();
            To = parsed.Groups["To"].Value.Trim();
        }

        public Joint(string f, string t)
        {
            From = f; To = t;
        }

        public override string ToString() => $"joint = {From} = {To}";
    }

    class JointRotationOffset
    {
        public string BoneName;
        public Quaternion offset;

        private Regex parseRx = new Regex(@"(?<BoneName>.*)\s*=\s*\(\s*(?<X>.*)\s*,\s*(?<Y>.*)\s*,\s*(?<Z>.*)\s*,\s*(?<W>.*)\s*\)", RegexOptions.Compiled | RegexOptions.IgnoreCase);

        public JointRotationOffset(string value)
        {
            var parsed = parseRx.Matches(value).First();
            BoneName = parsed.Groups["BoneName"].Value.Trim();
            offset = new Quaternion
            {
                x = float.Parse(parsed.Groups["X"].Value, CultureInfo.InvariantCulture),
                y = float.Parse(parsed.Groups["Y"].Value, CultureInfo.InvariantCulture),
                z = float.Parse(parsed.Groups["Z"].Value, CultureInfo.InvariantCulture),
                w = float.Parse(parsed.Groups["W"].Value, CultureInfo.InvariantCulture)
            };
        }

        public JointRotationOffset(string boneName, float x, float y, float z, float w)
        {
            BoneName = boneName;
            offset = new Quaternion(x, y, z, w);
        }

        public override string ToString() => $"jointRotationOffset2 = {BoneName} = ({offset.x.F()}, {offset.y.F()}, {offset.z.F()}, {offset.w.F()})";
    }

    class RemapBlendShape
    {
        public string From;
        public string To;
        public float Multiplier;

        private Regex parseRx = new Regex(@"(?<From>.*)\s*=\s*(?<To>.*)\s*=\s*(?<Multiplier>.*)", RegexOptions.Compiled | RegexOptions.IgnoreCase);

        public RemapBlendShape(string rawData)
        {
            var parsed = parseRx.Matches(rawData).First();
            From = parsed.Groups["From"].Value.Trim();
            To = parsed.Groups["To"].Value.Trim();
            Multiplier = float.Parse(parsed.Groups["Multiplier"].Value, CultureInfo.InvariantCulture);
        }

        public override string ToString() => $"bs = {From} = {To} = {Multiplier.F()}";
    }

    class JointIndex
    {
        public string BoneName;
        public int Index;

        private Regex parseRx = new Regex(@"^(?<BoneName>.*)\s*=\s*(?<Index>.*)", RegexOptions.Compiled | RegexOptions.IgnoreCase);

        public JointIndex(string rawData)
        {
            var parsed = parseRx.Matches(rawData).First();
            BoneName = parsed.Groups["BoneName"].Value.Trim();
            Index = int.Parse(parsed.Groups["Index"].Value);
        }
        
        public override string ToString() => $"jointIndex = {BoneName} = {Index}";
    }

    class FST
    {
        public readonly string exporterVersion = AvatarExporter.AVATAR_EXPORTER_VERSION;
        public string name;
        public string type = "body+head";
        public float scale = 1.0f;
        public string filename;
        public string texdir = "textures";
        public string materialMap;
        public string script;

        public List<RemapBlendShape> remapBlendShapeList = new List<RemapBlendShape>();

        public List<Joint> jointList = new List<Joint>();
        public List<JointMap> jointMapList = new List<JointMap>();
        public List<JointRotationOffset> jointRotationList = new List<JointRotationOffset>();
        public List<JointIndex> jointIndexList = new List<JointIndex>();
        public List<string> freeJointList = new List<string>();

        private Regex parseRx = new Regex(@"^(?<Key>[\w]*)\s*=\s*(?<Value>.*)", RegexOptions.Compiled | RegexOptions.IgnoreCase);

        public string flowPhysicsData;
        public string flowCollisionsData;

        public string lod;
        public string joint;

        List<string> fstContent = new List<string>();

        public bool ExportFile(string fstPath)
        {
            fstContent = new List<string>
            {
                $"exporterVersion = {exporterVersion}",
                $"name     = {name}",
                $"type     = {type}",
                $"scale    = {scale.F()}",
                $"filename = {filename}",
                $"texdir   = {texdir}"
            };
            AddIfNotNull(remapBlendShapeList);
            AddIfNotNull(jointMapList);
            AddIfNotNull(jointIndexList);
            AddIfNotNull(jointRotationList);
            AddIfNotNull("freeJoint", freeJointList);

            AddIfNotNull(nameof(materialMap), materialMap);
            AddIfNotNull(nameof(flowPhysicsData), flowPhysicsData);
            AddIfNotNull(nameof(flowCollisionsData), flowCollisionsData);
            AddIfNotNull(nameof(lod), lod);
            AddIfNotNull(nameof(joint), joint);
            AddIfNotNull(nameof(script), script);

            try
            {
                System.IO.File.WriteAllLines(fstPath, fstContent);
                return true;
            }
            catch (Exception e)
            {
                EditorUtility.DisplayDialog("Error", "Failed to write file " + fstPath +
                                            ". Please check the location and try again.", "Ok");
                Debug.LogException(e);
                return false;
            }
        }

        private void AddIfNotNull<T>(string keyname, List<T> list)
        {
            if (list.Count != 0)
                list.ForEach(x => fstContent.Add($"{keyname} = {x}"));
        }

        private void AddIfNotNull<T>(List<T> list)
        {
            if (list.Count != 0)
                fstContent.Add(string.Join("\n", list));
        }

        private void AddIfNotNull(string keyname, string valname)
        {
            if (!string.IsNullOrEmpty(valname))
                fstContent.Add($"{keyname} = {valname}");
        }


        public bool LoadFile(string fstPath)
        {
            try
            {
                var rawFst = System.IO.File.ReadAllLines(fstPath);

                foreach (var l in rawFst)
                {
                    var match = parseRx.Matches(l)[0];
                    ParseLine(match.Groups["Key"].Value.Trim(), match.Groups["Value"].Value.Trim());
                }

                return true;
            }
            catch (Exception e)
            {
                EditorUtility.DisplayDialog("Error", "Failed to read file " + fstPath +
                                            ". Please check the location and try again.", "Ok");
                Debug.LogException(e);
                return false;
            }
        }

        private void ParseLine(string key, string value)
        {
            switch (key)
            {
                case "exporterVersion":
                    //Just ingnore the old exporterVersion
                    break;
                case "name":
                    name = value;
                    break;
                case "type":
                    type = value;
                    break;
                case "scale":
                    scale = float.Parse(value, CultureInfo.InvariantCulture);
                    break;
                case "filename":
                    filename = value;
                    break;
                case "texdir":
                    texdir = value;
                    break;
                case "materialMap":
                    // The materialMap will be generated by unity, no need to parse it
                    // TODO:Parse it when changed to importing instead of updating
                    break;

                case "joint":
                    jointList.Add(new Joint(value));
                    break;
                case "jointMap":
                    jointMapList.Add(new JointMap(value));
                    break;
                case "jointRotationOffset2":
                    jointRotationList.Add(new JointRotationOffset(value));
                    break;
                case "jointIndex":
                    jointIndexList.Add(new JointIndex(value));
                    break;
                case "freeJoint":
                    freeJointList.Add(value);
                    break;

                case "bs":
                    remapBlendShapeList.Add(new RemapBlendShape(value));
                    break;

                default:
                    Debug.LogError($"Unknown key \"{key}\"\nPlease report this issue on the issue tracker");
                    break;
            }
        }

        private KeyValuePair<string, string> ParseKVPair(Regex rx, string sinput)
        {
            var match = rx.Matches(sinput).First();
            return new KeyValuePair<string, string>(match.Groups["Key"].Value.Trim(), match.Groups["Value"].Value.Trim());
        }
    }

    class UserBoneInformation
    {
        public string humanName; // bone name in Humanoid if it is mapped, otherwise ""
        public string parentName; // parent user bone name
        public BoneTreeNode boneTreeNode; // node within the user bone tree
        public int mappingCount; // number of times this bone is mapped in Humanoid
        public Vector3 position; // absolute position
        public Quaternion rotation; // absolute rotation

        public UserBoneInformation()
        {
            humanName = "";
            parentName = "";
            boneTreeNode = new BoneTreeNode();
            mappingCount = 0;
            position = new Vector3();
            rotation = new Quaternion();
        }
        public UserBoneInformation(string parent, BoneTreeNode treeNode, Vector3 pos)
        {
            humanName = "";
            parentName = parent;
            boneTreeNode = treeNode;
            mappingCount = 0;
            position = pos;
            rotation = new Quaternion();
        }

        public bool HasHumanMapping() { return !string.IsNullOrEmpty(humanName); }
    }

    class BoneTreeNode
    {
        public string boneName;
        public string parentName;
        public List<BoneTreeNode> children = new List<BoneTreeNode>();

        public BoneTreeNode() { }
        public BoneTreeNode(string name, string parent)
        {
            boneName = name;
            parentName = parent;
        }
    }

    class MaterialData
    {
        public Color albedo;
        public string albedoMap;
        public double metallic;
        public string metallicMap;
        public double roughness;
        public string roughnessMap;
        public string normalMap;
        public string occlusionMap;
        public Color emissive;
        public string emissiveMap;

        public override string ToString()
        {
            string json = "{ \"materialVersion\": 1, \"materials\": { ";

            //Albedo
            json += $"\"albedo\": [{albedo.r.F()}, {albedo.g.F()}, {albedo.b.F()}], ";
            if (!string.IsNullOrEmpty(albedoMap))
                json += $"\"albedoMap\": \"{albedoMap}\", ";

            //Metallic
            json += $"\"metallic\": {metallic.F()}, ";
            if (!string.IsNullOrEmpty(metallicMap))
                json += $"\"metallicMap\": \"{metallicMap}\", ";

            //Roughness
            json += $"\"roughness\": {roughness.F()}, ";
            if (!string.IsNullOrEmpty(roughnessMap))
                json += $"\"roughnessMap\": \"{roughnessMap}\", ";

            //Normal
            if (!string.IsNullOrEmpty(normalMap))
                json += $"\"normalMap\": \"{normalMap}\", ";

            //Occlusion
            if (!string.IsNullOrEmpty(occlusionMap))
                json += $"\"occlusionMap\": \"{occlusionMap}\", ";

            //Emissive
            json += $"\"emissive\": [{emissive.r.F()}, {emissive.g.F()}, {emissive.b.F()}]";
            if (!string.IsNullOrEmpty(emissiveMap))
                json += $", \"emissiveMap\": \"{emissiveMap}\"";

            json += " } }";
            return json;
        }
    }

    public static class ConverterExtensions
    {
        //Helper function to convert floats to string without commas
        public static string F(this float x) => x.ToString("G", CultureInfo.InvariantCulture);
        public static string F(this double x) => x.ToString("G", CultureInfo.InvariantCulture);
    }
}