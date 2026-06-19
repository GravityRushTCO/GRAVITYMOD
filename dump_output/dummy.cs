// Generated IL2CPP Dump
using System;
using System.Collections.Generic;

namespace OneState {
    public class PlayerData {
        public bool GetCareerStatus() { return false; }
        public int GetUserVirtualCurrency() { return 0; }
    }
    
    public class CarData {
        public bool UnlockCar() { return false; }
    }
    
    public class WeaponData {
        public void AddRecoilForce() { }
    }
    
    public class LevelData {
        public int get_Level() { return 1; }
        public int get_Rank() { return 1; }
    }
    
    public class VIPData {
        public bool IsVIP() { return false; }
    }
}

namespace Game {
    public class Player {
        public bool IsVIP() { return false; }
        public int GetCash() { return 0; }
        public int GetLevel() { return 1; }
        public int GetRank() { return 1; }
    }
}
