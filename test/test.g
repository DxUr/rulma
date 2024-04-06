class_name MovementController
extends Node

@export var character: Character
@export var animation_controller: AnimationController

@export var speed: float = 100.0
@export var acc: float = 50.0
@export var dec: float = 50.0


enum State {
	IDLE = 0,
	RUN_FORWARD,
	RUN_BACKWARD,
}

func _get_dir() -> Character.Direction:
	if character.moving_dir.x > 0:
		return Character.Direction.RIGHT
	return Character.Direction.LEFT

func _is_running_forward() -> bool:
	return _get_dir() == character.looking_dir

func move(p_dir: Vector2):
	character.moving_dir = p_dir

func look_to(p_point: Vector2) -> void:
	character.looking_dir = Character.Direction.LEFT if character.body.global_position.direction_to(p_point).x < 0.0 else Character.Direction.RIGHT
	character.sprite.flip_h = character.looking_dir == Character.Direction.LEFT
	character.hand.position.x = (abs(character.hand.position.x) if
		character.looking_dir == Character.Direction.RIGHT else
			-abs(character.hand.position.x))

func idle_state(_delta: float) -> void:
	if character.moving_dir: run_state(_delta)

func run_state(delta: float) -> void:
	character.body.velocity = character.body.velocity.move_toward(
		Util.vec_norm_dir(character.moving_dir) * speed,
			(acc if character.moving_dir else dec) * speed * delta)
	character.body.move_and_slide()

	if _is_running_forward():
		character.set_state(State.RUN_FORWARD)
	else:
		character.set_state(State.RUN_BACKWARD)
	if character.body.velocity.is_zero_approx():
		character.set_state(State.IDLE)

func _physics_process(delta: float) -> void:
	match character.state:
		State.IDLE:
			idle_state(delta)
		State.RUN_FORWARD, State.RUN_BACKWARD:
			run_state(delta)

func _init(p_character: Character = null) -> void:
	character = p_character

func _ready() -> void:
	assert(character)
	assert(animation_controller)

	animation_controller.set_state_anim(State.IDLE, &"IDLE")
	animation_controller.set_state_anim(State.RUN_FORWARD, &"RUN")
	animation_controller.set_state_anim(State.RUN_BACKWARD, &"RUN", true)

