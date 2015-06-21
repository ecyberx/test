#pragma strict

var Accel = 100.0;

function Start () {
}

function Update () {
	GetComponent.<Rigidbody>().AddForce(
		transform.right * Input.GetAxisRaw( "Horizontal" ) *Accel,
		ForceMode.Impulse
	);

}