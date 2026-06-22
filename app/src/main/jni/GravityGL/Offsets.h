#pragma once

// Automatically generated - V18 RVAs verified against dump_v18.cs
// Last updated: 2026-06-12 (Corrected erroneous UnityEngine and Physics shifts)

#define RVA_NetworkVehicle_Extrapolate 0x0 /* NOT FOUND */
#define RVA_VIP_GetLevel 0x0               /* NOT FOUND */
#define RVA_Player_GetLevel 0x0            /* NOT FOUND */
#define RVA_Player_NotKo 0x0               /* NOT FOUND */
#define RVA_SetVelocity 0X326C2C4

// CharacterActor.SweepAndTeleport(Vector3) @ 0x3265D98
#define RVA_SweepAndTeleport 0X3265D98

// Animator.GetBoneTransformInternal @ 0x428BD08
#define RVA_Animator_GetBoneTransformInternal 0X428BD08

// Rigidbody.set_velocity_Injected @ 0x4369540
#define RVA_Rigidbody_SetVelocity 0X4369540

// Rigidbody.set_angularVelocity_Injected @ 0x4369678
#define RVA_Rigidbody_SetAngularVelocity 0X4369678

// Rigidbody.set_isKinematic @ 0x43699E4 (Unity Rigidbody, not ECS)
#define RVA_SetIsKinematic 0X43699E4

// Rigidbody.WakeUp() @ 0x436A8CC — Réveille un rigidbody endormi après
// téléportation
#define RVA_Rigidbody_WakeUp 0X436A8CC

// SetHealth(float value) @ 0x2ACA0EC — Force la santé du joueur local
#define RVA_SetHealth 0X2ACA0EC

// Camera.set_fieldOfView @ 0x42A9974 ✓
#define RVA_SetFieldOfView 0X42A9974

// Camera.FireOnPreCull @ 0x42AE5A8 ✓
#define RVA_FireOnPreCull 0X42AE5A8

// SystemInfo.get_deviceModel @ 0x4301C20
#define RVA_GetDeviceModel 0x4301C20

// SystemInfo.get_deviceName @ 0x4301BD0
#define RVA_GetDeviceName 0x4301BD0

#define RVA_Joystick_GetHorizontal 0x230EB34
#define RVA_Joystick_GetVertical 0x230EB1C
#define RVA_WeaponAttackNotAllowedDatagram_Read 0x2BE00C0
#define RVA_WeaponAttackEmitDatagram_Write 0x2BDF918
#define RVA_CasinoShopBuyRequestDatagram_Write 0x29FAE84
#define RVA_UpdatePath 0x312F078
#define RVA_SetMapPosition 0x2515984
#define RVA_CreateMapCheckpointSignalHandler 0x251D674
#define RVA_CreateMapGpsCheckpoint_ctor 0x282A168
#define RVA_VehicleOcclusionObject_OnEnable 0X310D944
#define RVA_VehicleOcclusionObject_OnDisable 0X310DA94
#define RVA_TogglePlayerMarker 0x2B19510
#define RVA_FreeCamHandler 0x2AAA784
#define RVA_NicknameTextDatagram_Read 0x2737038
#define RVA_RayCastRanged 0x2BD71F8
#define RVA_PedDisplacementSyncDatagram_Read 0x2738D48
#define RVA_VehicleDisplacementSyncDatagram_Read 0x2738DF8
#define RVA_ElementHealthDatagram_Read 0x273831C
#define RVA_ElementArmorDatagram_Read 0x2737DCC

// Component.GetComponent(string) @ 0x42F5AAC ✓
#define RVA_GetComponentString 0X42F5AAC

// Transform.set_position_Injected @ 0x43058A8
#define RVA_Transform_SetPosition 0X43058A8

// Transform.get_position_Injected @ 0x4305810
#define RVA_Transform_GetPosition 0X4305810

// Transform.get_parent @ 0x43056B8 ✓
#define RVA_Transform_GetParent 0X43056B8

// Transform.get_root @ 0x4308330 ✓
#define RVA_Transform_GetRoot 0X4308330

// Transform.get_childCount @ 0x43083A8 ✓
#define RVA_Transform_GetChildCount 0X43083A8

// Transform.GetChild(int) @ 0x43089FC ✓
#define RVA_Transform_GetChild 0X43089FC

// Physics.SyncTransforms @ 0x43674D0 ✓
#define RVA_Physics_SyncTransforms 0X43674D0

// Component.get_transform @ 0x42F5928 ✓
#define RVA_Component_GetTransform 0X42F5928

// Transform.Internal_LookAt_Injected @ 0x43071BC ✓ (makes transform look at
// world pos)
#define RVA_Transform_LookAt_Injected 0X43071BC

// MainCameraStorage.SetCameraLookDirection @ 0x31BDF64
#define RVA_SetCameraLookDirection 0x31BDF64

// Transform.set_rotation_Injected @ 0x4306068 ✓
#define RVA_Transform_SetRotation_Injected 0X4306068

// Rigidbody.set_position_Injected @ 0x436A260
#define RVA_Rigidbody_SetPosition 0X436A260

#define RVA_Knockout1 0x0
#define RVA_KoState 0x0
#define RVA_InternalHitFilter 0x0
#define RVA_SetPlanarVelocity 0X326C36C
#define RVA_GuiHudMap_OnPointerUp 0x0

// CharacterActor.Teleport(Vector3) @ 0x326CA10
#define RVA_CharacterActor_Teleport 0X326CA10

// HudMapNavigationSystem.FindGoal @ 0x2516B34
#define RVA_FindGoal 0X2516B34

#define RVA_Object_GetName 0X42FDD8C
#define RVA_Camera_GetMain 0X42ACEC0
#define RVA_Camera_GetWorldToCameraMatrix 0X42ABD2C
#define RVA_Camera_GetProjectionMatrix 0X42ABE64
#define RVA_Camera_GetPixelWidth 0X42AB848
#define RVA_Camera_GetPixelHeight 0X42AB884
#define RVA_Object_op_Implicit 0x42FDC1C
#define RVA_Camera_ResetWorldToCameraMatrix 0X42AC198

// HudMapNavigationSystem.FindGoal @ 0x2516B34 (déjà en Main.cpp, dupliqué ici
// pour cohérence) CharacterActor.set_alwaysNotGrounded est un champ inline —
// pas de RVA séparé Offset dans CharacterActor : 0xEC (alwaysNotGrounded),
// 0x1A0 (isGrounded), 0x1A1 (isStable)