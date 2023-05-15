import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { SetupWizardComponent } from './config/setupwizard/setupwizard.component';
import { DashboardComponent } from './dashboard/dashboard.component';
import { GuideComponent } from './guide/guide.component';
import { StatusComponent } from './status/status.component';
import { SettingsComponent } from './config/settings/general/general-settings.component';
import { CanDeactivateGuardService } from './can-deactivate-guard.service';
import { CaptureCardsComponent } from './config/settings/capture-cards/capture-cards.component';
import { VideoSourcesComponent } from './config/settings/video-sources/video-sources.component';
import { InputConnectionsComponent } from './config/settings/input-connections/input-connections.component';
import { StorageGroupsComponent } from './config/settings/storage-groups/storage-groups.component';
import { ChannelEditorComponent } from './config/settings/channel-editor/channel-editor.component';
import { RecordingProfilesComponent } from './config/settings/recording-profiles/recording-profiles.component';
import { SystemEventsComponent } from './config/settings/system-events/system-events.component';

const routes: Routes = [
  { path: 'dashboard', component: DashboardComponent },
  { path: 'setupwizard', component: SetupWizardComponent },
];

@NgModule({
  imports: [RouterModule.forRoot(routes)],
  exports: [RouterModule],
  providers: [CanDeactivateGuardService]
})
export class AppRoutingModule { }
