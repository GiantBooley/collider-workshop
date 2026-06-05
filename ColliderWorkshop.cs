using System.Collections;
using System.Collections.Generic;
using System.IO;
using UnityEngine;
using UnityEditor;
using System.Linq;

public class ColliderWorkshop : MonoBehaviour {

    /*

serial v3 specification
{
    "projectAssetsPath": "x/Assets",
    "sprites": [
        {
            "objectPath": "Terrain/rock",
            "texturePath": "Sprites/rock.png",
            "rectMinX": 0,
            "rectMinY": 0,
            "rectMaxX": 1024,
            "rectMaxY": 1024,
            "posX": 0,
            "posY": 0,
            "posZ": 0,
            "rotX": 0,
            "rotY": 0,
            "rotZ": 0,
            "scaX": 1,
            "scaY": 1,
            "scaZ": 1,
            "pixelsPerUnit": 100.0,
            "sortingOrder": 0,
            "colliders": [
                {
                    "xPositions": [],
                    "yPositions": [],
                    "physicsMaterialAssetPath": "GOIWBF Level Tools/...",
                    "soundMaterial": 4,
                    "isCustomHitSound": false,
                    "customHitSoundName": "Pumpkin",
                    "r": 126,
                    "g": 126,
                    "b": 126
                }
                ...
            ]
        }
        ...
    ],
    "customHitSounds": [
        {
            "name": "Pumpkin"
        }
        ...
    ],
    "physicsMaterials": [
        "assetPath": "GOIWBF Level Tools/...",
        "friction": 10.0,
        "bounciness": 0.0
    ]
}

    */

    [System.Serializable]
    private class CWCollider {
        public List<float> xPositions;
        public List<float> yPositions;
        public string physicsMaterialAssetPath;
        public int soundMaterial;
        public bool isCustomHitSound;
        public string customHitSoundName;
        public int r;
        public int g;
        public int b;
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
        public string projectAssetsPath;
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
    [MenuItem("Tools/Export collider workshop")]
    private static void serialize() {
        string projectAssetsPath = Application.dataPath;

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
            float minY = spriteRenderer.sprite.rect.yMin;
            float maxY = spriteRenderer.sprite.rect.yMax;
            float originalMinY = minY;
            minY = spriteRenderer.sprite.texture.height - maxY;
            maxY = spriteRenderer.sprite.texture.height - originalMinY;
            CWSprite sprite = new CWSprite{
                objectPath = GetGameObjectPath(spriteRenderer.gameObject),
                texturePath = texturePath,
                rectMinX = spriteRenderer.sprite.rect.xMin,
                rectMinY = minY,
                rectMaxX = spriteRenderer.sprite.rect.xMax,
                rectMaxY = maxY,
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
            projectAssetsPath = projectAssetsPath,
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

    private static void RemoveCollidersFromGameObject(GameObject g) {
        Collider2D[] colliders = g.GetComponents<Collider2D>();
        foreach (Collider2D collider in colliders) {
            DestroyImmediate(collider);
        }
        GroundCol[] groundCols = g.GetComponents<GroundCol>();
        foreach (GroundCol groundCol in groundCols) {
            DestroyImmediate(groundCol);
        }
        CustomGroundCol[] customGroundCols = g.GetComponents<CustomGroundCol>();
        foreach (CustomGroundCol customGroundCol in customGroundCols) {
            DestroyImmediate(customGroundCol);
        }
    }

    [MenuItem("Tools/Import collider workshop")]
    private static void deserialize() {
        try {
            using StreamReader reader = new StreamReader("Assets/colliderworkshop_unity.json");
            string text = reader.ReadToEnd();

            CWSerial serialObject = JsonUtility.FromJson<CWSerial>(text);

            foreach (CWSprite sprite in serialObject.sprites) {
                // Step 1: delete colliders already on sprite ================
                GameObject g = GameObject.Find(sprite.objectPath);

                RemoveCollidersFromGameObject(g);

                int childCount = g.transform.childCount;
                for (int j = childCount - 1; j >= 0; j--) {
                    Transform child = g.transform.GetChild(j);
                    RemoveCollidersFromGameObject(child.gameObject);

                    if (child.GetComponents<Component>().Length <= 1) {
                        DestroyImmediate(child.gameObject);
                    }
                }

                // Step 2: add colliders =====================================
                int i = 1;
                foreach (CWCollider collider in sprite.colliders) {
                    // create collider object
                    GameObject go = new GameObject("Collider" + i);
                    go.transform.SetParent(g.transform, false);
                    go.layer = 10; // set to Terrain layer
                    i++;

                    // add PolygonCollider2D and set path
                    PolygonCollider2D colliderComp = go.AddComponent<PolygonCollider2D>();
                    List<Vector2> points = new List<Vector2>();
                    for (int j = 0; j < collider.xPositions.Count; j++) {
                        points.Add(new Vector2(collider.xPositions[j], collider.yPositions[j]));
                    }
                    colliderComp.SetPath(0, points);

                    // set physics material 2d
                    PhysicsMaterial2D mat = AssetDatabase.LoadAssetAtPath<PhysicsMaterial2D>("Assets/" + collider.physicsMaterialAssetPath);
                    colliderComp.sharedMaterial = mat;

                    // add GroundCol or CustomGroundCol
                    if (collider.isCustomHitSound) {
                        CustomGroundCol customGroundCol = go.AddComponent<CustomGroundCol>();
                        customGroundCol.groundCol = new Color((float)collider.r / 255f, (float)collider.g / 255f, (float)collider.b / 255f);
                        customGroundCol.material = collider.customHitSoundName;
                        customGroundCol.isSolid = false; // todo: add setting
                    } else {
                        GroundCol groundCol = go.AddComponent<GroundCol>();
                        groundCol.groundCol = new Color((float)collider.r / 255f, (float)collider.g / 255f, (float)collider.b / 255f);
                        groundCol.material = (GroundCol.SoundMaterial)collider.soundMaterial;
                    }
                }
            }

        } catch (IOException e) {
            Debug.LogError("The json file could not be read:");
            Debug.LogError(e.Message);
        }
    }
}
