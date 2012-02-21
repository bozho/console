#pragma once
class JumpList
{
public:
  static void CreateList(TabDataVector &tabDataVector);
  static void Activate(void);
private:
  static bool bActive;
};

