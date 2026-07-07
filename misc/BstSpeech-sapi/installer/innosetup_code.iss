var
  DonatePage: TWizardPage;
  DonateLabel: TNewStaticText;
  DonateLabel2: TNewStaticText;
  PayPalButton: TNewButton;

procedure PayPalButtonClick(Sender: TObject);
var
  ErrorCode: Integer;
begin
  ShellExec('open', 'https://paypal.me/gozaltech', '', '', SW_SHOWNORMAL, ewNoWait, ErrorCode);
end;

procedure CreateDonatePage;
begin
  DonatePage := CreateCustomPage(wpWelcome, 'Support Development', 'Help improve Bestspeech SAPI');

  DonateLabel := TNewStaticText.Create(DonatePage);
  DonateLabel.Parent := DonatePage.Surface;
  DonateLabel.Caption := 'Thank you for installing Bestspeech SAPI!';
  DonateLabel.Left := 0;
  DonateLabel.Top := 0;
  DonateLabel.Width := DonatePage.SurfaceWidth;
  DonateLabel.Height := 40;
  DonateLabel.Font.Size := 12;
  DonateLabel.Font.Style := [fsBold];

  DonateLabel2 := TNewStaticText.Create(DonatePage);
  DonateLabel2.Parent := DonatePage.Surface;
  DonateLabel2.Caption := 'This software is provided free of charge. If you find it useful, please consider supporting development with a donation.' + #13#10 + #13#10 + 'Your support helps us continue improving this project.';
  DonateLabel2.Left := 0;
  DonateLabel2.Top := 50;
  DonateLabel2.Width := DonatePage.SurfaceWidth;
  DonateLabel2.Height := 80;
  DonateLabel2.AutoSize := False;
  DonateLabel2.WordWrap := True;

  PayPalButton := TNewButton.Create(DonatePage);
  PayPalButton.Parent := DonatePage.Surface;
  PayPalButton.Caption := 'Donate via PayPal';
  PayPalButton.Left := (DonatePage.SurfaceWidth - 150) div 2;
  PayPalButton.Top := 140;
  PayPalButton.Width := 150;
  PayPalButton.Height := 30;
  PayPalButton.OnClick := @PayPalButtonClick;
end;

procedure InitializeWizard();
begin
  CreateDonatePage;
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  ResultCode: Integer;
begin
  if CurStep = ssPostInstall then
  begin
    if not Exec(ExpandConstant('{sys}\regsvr32.exe'), '/s "' + ExpandConstant('{app}') + '\BestspeechSAPI.dll"', '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
      MsgBox('Failed to register x86 DLL', mbError, MB_OK);

    if IsWin64 and FileExists(ExpandConstant('{app}\x64\BestspeechSAPI.dll')) then
    begin
      if not Exec(ExpandConstant('{syswow64}\regsvr32.exe'), '/s "' + ExpandConstant('{app}') + '\x64\BestspeechSAPI.dll"', '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
        MsgBox('Failed to register x64 DLL', mbError, MB_OK);
    end;
  end;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  ResultCode: Integer;
begin
  if CurUninstallStep = usUninstall then
  begin
    Exec(ExpandConstant('{sys}\regsvr32.exe'), '/s /u "' + ExpandConstant('{app}') + '\BestspeechSAPI.dll"', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);

    if IsWin64 and FileExists(ExpandConstant('{app}\x64\BestspeechSAPI.dll')) then
    begin
      Exec(ExpandConstant('{syswow64}\regsvr32.exe'), '/s /u "' + ExpandConstant('{app}') + '\x64\BestspeechSAPI.dll"', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
    end;

    Sleep(2000);
  end;
end;
