using UnityEngine;
using System.Collections;

public class PlayerController : MonoBehaviour {


	public float speed;
	public float speed_key;

	private Rigidbody rb;

	void Start()
	{
		rb=GetComponent<Rigidbody>();
	}
	void FixedUpdate()
	{
#if !UNITY_ANDROID
		Vector3 dir = Vector3.zero;
		dir.x = Input.acceleration.x;
		dir.z = Input.acceleration.y;
		if(dir.sqrMagnitude>1){dir.Normalize ();}
		dir *= Time.deltaTime;
		
		rb.AddForce (dir*speed);
#else
		float moveHorizontal = Input.GetAxis ("Horizontal");
		float moveVertical = Input.GetAxis ("Vertical");
		Vector3 movement = new Vector3 (moveHorizontal, 0.0f, moveVertical);
		rb.AddForce (movement*speed_key);

#endif

	}
	void OnTriggerEnter(Collider other) 
	{
		if (other.gameObject.CompareTag ("Pick Up")) {
			other.gameObject.SetActive (false);
		}
	}
}
