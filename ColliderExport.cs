#if UNITY_EDITOR
using System.IO;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using System.Linq;

public class ColliderExport : Editor
{
    [MenuItem("Assets/Export Colliders")]
    static void Do()
    {
        // Settings
        string parentNameWithoutSlashAfter = "/Terrain";
        string collidersTextFilePath = Path.Combine(Application.dataPath, "colliders.txt");



        GameObject parent = GameObject.Find(parentNameWithoutSlashAfter);
        int howmany = parent.transform.childCount;
        using (StreamWriter sw = File.CreateText(collidersTextFilePath)) {
            for (int i = 0; i < howmany; i++) { // for each object
                GameObject spriteObject = parent.transform.GetChild(i).gameObject;
                SpriteRenderer spriteComponent = spriteObject.GetComponent<SpriteRenderer>();
                if (spriteComponent != null && spriteComponent.sprite != null) { // if it has a sprite renderer
                    sw.WriteLine(parentNameWithoutSlashAfter + "/" + spriteObject.name); // object path
					string dataPath = Application.dataPath;
					dataPath = dataPath.Substring(0, dataPath.Length - 6);//remove assets and stuff
                    sw.WriteLine(dataPath + AssetDatabase.GetAssetPath(spriteComponent.sprite)); // image path
                    // bounds
                    Rect rect = spriteComponent.sprite.textureRect;
                    sw.WriteLine(rect.xMin.ToString() + "," + rect.yMin.ToString() + "," + rect.xMax.ToString() + "," + rect.yMax.ToString());
                }
            }
        }
        AssetDatabase.Refresh();
    }
}
#endif
