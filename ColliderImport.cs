#if UNITY_EDITOR
using System.IO;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using System.Linq;
using System;

public class ColliderImport : Editor
{
    [MenuItem("Assets/Import Colliders")]
    static void Do()
    {
        FileInfo theSourceFile = null;
        StreamReader reader = null;
        string text = " "; // assigned to allow first line to be read below

        theSourceFile = new FileInfo("D:/html/colliderworkshop/colliders_for_unity.txt");
        reader = theSourceFile.OpenText();

        int howman = -1;
        int colliderNumber = 1;
        GameObject obj = null;
        bool isCustomHitSound = false;
        string customHitSound = "";
        int hitSound = 0;
        string frictionName = "";
        int line = 0;
        while ((text = reader.ReadLine()) != null) { // for each line
            if (howman == -1) { // object name
                if (text == "polygon") {
                    howman = 1;
                    continue;
                }

                colliderNumber = 1;
                obj = GameObject.Find(text);
                // remove every collider
                PolygonCollider2D polygon = obj.GetComponent<PolygonCollider2D>();
                if (polygon != null) {
                    DestroyImmediate(polygon);
                }
                int howmanychild = obj.transform.childCount;
                for (int i = howmanychild - 1; i >= 0; i--) { // for each child
                    GameObject childObject = obj.transform.GetChild(i).gameObject;
                    PolygonCollider2D childPolygon = childObject.GetComponent<PolygonCollider2D>();
                    if (childPolygon != null && !childPolygon.isTrigger) { // if it has a collider and not trigger
                        DestroyImmediate(childObject);
                    }
                }
            }

            // howman 0 is polygon
            if (howman == 1) { // is custom hit sound
                isCustomHitSound = (text == "1");
            }
            if (howman == 2) { // hit sound
                if (isCustomHitSound) {
                    customHitSound = text;
                } else {
                    hitSound = Int32.Parse(text);
                }
            }
            if (howman == 3) { // friction
                frictionName = text;
            }
            if (howman == 4) { // points
                int ecksOrWhy = 0; // 0 x 1 y
                string xString = "";
                string yString = "";

                GameObject collider = new GameObject("Collider" + colliderNumber, typeof(PolygonCollider2D));
                colliderNumber++;
                collider.transform.parent = obj.transform;
                collider.transform.localPosition = new Vector3(0, 0, 0);
                collider.transform.localRotation = Quaternion.Euler(0, 0, 0);
                collider.transform.localScale = new Vector3(1, 1, 1);
                List<Vector2> points = new List<Vector2>();


                foreach (char c in text) {
                    if (ecksOrWhy == 0) { //x
                        if (c == 'x') {
                            ecksOrWhy = 1;
                        } else {
                            xString += c;
                        }
                    } else { //y
                        if (c == 'y') {
                            ecksOrWhy = 0;
                            points.Add(new Vector2(float.Parse(xString), float.Parse(yString)));
                            xString = "";
                            yString = "";
                        } else {
                            yString += c;
                        }
                    }
                }
                collider.GetComponent<PolygonCollider2D>().SetPath(0, points);
                if (isCustomHitSound) {
                    CustomGroundCol col = collider.AddComponent<CustomGroundCol>();
                    col.material = customHitSound;
                } else {
                    GroundCol col = collider.AddComponent<GroundCol>();
                    col.material = (GroundCol.SoundMaterial)hitSound;
                }
                PhysicsMaterial2D physicsMaterial = AssetDatabase.LoadAssetAtPath<PhysicsMaterial2D>("Assets/GOIWBF Level Tools/Tool/Scripts/" + frictionName + ".asset");
                collider.GetComponent<PolygonCollider2D>().sharedMaterial = physicsMaterial;
                howman = -2;
            }
            howman++;
            line++;
        }
    }
}
#endif