using System.Collections;
using System.Collections.Generic;
using System.IO;
using UnityEngine;
using UnityEditor;
using System.Linq;

public class ColliderSerialize : MonoBehaviour {
    [System.Serializable]
    private class CWCollider {
        public List<Vector2> points;
        public string frictionAsset;
    }
    [System.Serializable]
    private class CWSprite {
        public string objectPath;
        public string texturePath;
        public float rectMinX;
        public float rectMinY;
        public float rectMaxX;
        public float rectMaxY;
        public float posX;
        public float posY;
        public float posZ;
        public float rotX;
        public float rotY;
        public float rotZ;
        public float scaX;
        public float scaY;
        public float scaZ;
        public float pixelsPerUnit;
        public int sortingOrder;
        public List<CWCollider> colliders;
    }
    [System.Serializable]
    private class CWCustomHitSound {
        public string name;
        // add sound files
    }
    [System.Serializable]
    private class CWPhysicsMaterial {
        public string assetPath;
        public float friction;
        public float bounciness;
    }
    [System.Serializable]
    private class CWSerial {
        public string projectPath;
        public List<CWSprite> sprites;
        public List<CWCustomHitSound> customHitSounds;
        public List<CWPhysicsMaterial> physicsMaterials;
    }
    private static string GetGameObjectPath(GameObject obj) {
        string path = "/" + obj.name;
        while (obj.transform.parent != null) {
            obj = obj.transform.parent.gameObject;
            path = "/" + obj.name + path;
        }
        return path;
    }
    private static string RemoveAssetsFromPath(string path) {
        if (path.StartsWith("Assets/") || path.StartsWith("Assets\\")) {
            return path.Substring(7);
        }
        return path;
    }
    [MenuItem("Tools/Serialize collider workshop")]
    private static void serialize() {
        string projectPath = Application.dataPath;

        // get custom hit sounds
        CustomHitSoundProvider soundProvider = Object.FindObjectOfType<CustomHitSoundProvider>();
        List<CWCustomHitSound> customHitSounds = new List<CWCustomHitSound>();
        if (soundProvider != null) {
            foreach (CustomHitSound sound in soundProvider.hitSounds) {
                customHitSounds.Add(new CWCustomHitSound{
                    name = sound.name
                });
            }
        }

        // get frictions
        PhysicsMaterial2D[] materials = AssetDatabase.FindAssets("t:PhysicsMaterial2D")
            .Select(guid => AssetDatabase.LoadAssetAtPath<PhysicsMaterial2D>(AssetDatabase.GUIDToAssetPath(guid)))
            .ToArray();
        List<CWPhysicsMaterial> physicsMaterials = new List<CWPhysicsMaterial>();
        foreach (PhysicsMaterial2D material in materials) {
            physicsMaterials.Add(new CWPhysicsMaterial{
                assetPath = RemoveAssetsFromPath(AssetDatabase.GetAssetPath(material)),
                friction = material.friction,
                bounciness = material.bounciness
            });
        }


        // add sprites
        SpriteRenderer[] spriteRenderers = Selection.gameObjects.Length > 0 ? Selection.gameObjects.SelectMany(go => go.GetComponentsInChildren<SpriteRenderer>()).ToArray() : Object.FindObjectsOfType<SpriteRenderer>();
        
        List<CWSprite> sprites = new List<CWSprite>();
        foreach (SpriteRenderer spriteRenderer in spriteRenderers) {
            // discard if not in terrain layer
            if (spriteRenderer.gameObject.layer != 10) continue;

            string texturePath = RemoveAssetsFromPath(AssetDatabase.GetAssetPath(spriteRenderer.sprite.texture));
            CWSprite sprite = new CWSprite{
                objectPath = GetGameObjectPath(spriteRenderer.gameObject),
                texturePath = texturePath,
                rectMinX = spriteRenderer.sprite.rect.xMin,
                rectMinY = spriteRenderer.sprite.rect.yMin,
                rectMaxX = spriteRenderer.sprite.rect.xMax,
                rectMaxY = spriteRenderer.sprite.rect.yMax,
                posX = spriteRenderer.gameObject.transform.position.x,
                posY = spriteRenderer.gameObject.transform.position.y,
                posZ = spriteRenderer.gameObject.transform.position.z,
                rotX = spriteRenderer.gameObject.transform.eulerAngles.x,
                rotY = spriteRenderer.gameObject.transform.eulerAngles.y,
                rotZ = spriteRenderer.gameObject.transform.eulerAngles.z,
                scaX = spriteRenderer.gameObject.transform.lossyScale.x,
                scaY = spriteRenderer.gameObject.transform.lossyScale.y,
                scaZ = spriteRenderer.gameObject.transform.lossyScale.z,
                pixelsPerUnit = spriteRenderer.sprite.pixelsPerUnit,
                sortingOrder = spriteRenderer.sortingOrder
            };
            sprites.Add(sprite);
        }
        CWSerial serialObject = new CWSerial{
            projectPath = projectPath,
            sprites = sprites,
            customHitSounds = customHitSounds,
            physicsMaterials = physicsMaterials
        };

        string serial = JsonUtility.ToJson(serialObject);

        // write file
        StreamWriter writer = new StreamWriter("Assets/colliderworkshop.json", false);
        writer.WriteLine(serial);
        writer.Close();
        AssetDatabase.Refresh();
        Debug.Log($"Saved collider workshop file: {sprites.Count} sprites");
    }
}
